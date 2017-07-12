import argparse
import random
import numpy as np

FLAGS = None

def filename (pre, chip, core, suffix='script'):
    return pre + str(chip) + '_' + str(core) + '.' + suffix

def generate_spike ():
    num_neurons = FLAGS.neurons
    rate = FLAGS.spike_rate / 10000.
    if(rate > 1):
        raise ValueError ("Maximum rate is 1.0")

    spike_train = []

    if (FLAGS.distribution == 'uniform'):
        for i in range(num_neurons):
            if (random.random() < rate):
                spike_train.append(i)

    return spike_train


def spike_generator():
    random.seed(706)
    for i in range (FLAGS.chip):
        for j in range (FLAGS.core):
            f = open(FLAGS.path + filename('data/spikes/spike',i,j), 'w')
            f.write("#!Tsim Script\n\n")
            f.write("CLASSNAME: SpikeFileScript\n")
            f.write("SCRIPT:\n")

            for t in range(FLAGS.timestep):
                spike_train = generate_spike()
                s = ''
                for spk in spike_train:
                    s += str(spk) + '/'
                s = s.strip('/')

                f.write ("\tsection\n")
                if(len(spike_train) > 0):
                    f.write("\t\tcycle=0, data=" + s)
                f.write("\n\tendsection\n\n")
            
            f.close()

def generate_syn (length):
    syn_data = np.random.permutation (FLAGS.chip * FLAGS.core * FLAGS.neurons)
    syn_data = syn_data[:length]
    syn_data.sort()

    dram_data = []
    for s in syn_data:
        neuron_idx = s % FLAGS.neurons
        core_idx = (s / FLAGS.neurons) % FLAGS.core
        chip_idx = (s / (FLAGS.neurons * FLAGS.core)) % FLAGS.chip

        weight = int(random.random() * (1<<16))

        data = (1 << 63) | (weight << 21) | (chip_idx << 18) | (core_idx << 15) | neuron_idx
        dram_data.append(data)

    return dram_data

def generate_meta(last_addr):
    num_neurons = FLAGS.neurons * FLAGS.core * FLAGS.chip
    sparsity = FLAGS.sparsity
    
    meta_table = []
    dram_data = []

    dram_idx = (last_addr / FLAGS.dram_size) + 1

    for n in range(FLAGS.neurons):
        if(dram_idx > FLAGS.propagator):
            raise ValueError ("Small dram size")
       
        length = np.random.binomial(num_neurons, sparsity)
        if ((last_addr + length) > dram_idx*FLAGS.dram_size):
            offset = (dram_idx * FLAGS.dram_size) - last_addr
            print ("(nidx: %d) last addr %d, offset %d, len %d" \
                    %(n, last_addr, offset, length))

            last_addr += offset
            for o in range(offset):
                dram_data.append(0)

            dram_idx += 1
        
        meta_table.append((last_addr, length))

        dram_data += generate_syn (length) 

        last_addr += length

    return meta_table, last_addr, dram_data

     

def meta_generator():
    np.random.seed(946)

    num_neurons = FLAGS.neurons * FLAGS.core * FLAGS.chip

    last_addr = 0

    dram_info = []
    for i in range (FLAGS.chip):
        for j in range (FLAGS.core):
            print ("chip %d core %d" %(i, j))
            f = open(FLAGS.path + filename('data/meta/meta',i,j), 'w')
            

            f.write("#!Tsim Script\n\n")
            f.write("CLASSNAME: MetaFileRegister\n")
            f.write("DATA:\n")
            
            meta, last_addr, dram = generate_meta(last_addr)
            dram_info += dram

            for idx, m in enumerate(meta):
                addr, length = m

                d = (addr << 16) | (length)
                f.write("\taddr=0x%04x, data=0x%016x\n" %(idx, d))

            f.close()

    # dram_size = 1
    # while (dram_size < (len(dram_info) // FLAGS.propagator)):
    #     dram_size = dram_size * 2
    # FLAGS.dram_size = dram_size

    print ("Dram size: %d (require: %d)" %(FLAGS.dram_size, len(dram_info)//FLAGS.propagator))

    if(len(dram_info) > FLAGS.dram_size*FLAGS.propagator):
        raise ValueError ("dram_size error %d" %(len(dram_info)))

    for i in range(FLAGS.propagator):
        f = open(FLAGS.path + 'data/dram/dram' + str(i) + '.script', 'w')
        f.write("#!Tsim Script\n\n")
        f.write("CLASSNAME: DramFileRegister\n")
        f.write("DATA:\n")

        for addr in range(FLAGS.dram_size):
            idx = i*FLAGS.dram_size + addr
            if(idx < len(dram_info)):
                f.write("\taddr=0x%04x, data=0x%016x\n" %(addr, dram_info[idx]))
            else:
                f.write("\taddr=0x%04x, data=0x%016x\n" %(addr, 0))

        f.close() 

def spec_generator():
    f = open("simspec/neurosim.simspec", 'w')

    f.write ("#!Tsim Simulation Spec\n\n")

    f.write ("# External file\n")
    for i in range(FLAGS.chip):
        for j in range(FLAGS.core):
            f.write("FILESCRIPT_PATH(top.chip%d.core%d.neuron_block): data/spikes/spike%d_%d.script\n" \
                    %(i, j, i, j))

    f.write ("\n")
    for i in range(FLAGS.chip):
        for j in range(FLAGS.core):
            f.write("REGISTER_DATAPATH(top.chip%d.core%d.axon_meta_table): data/meta/meta%d_%d.script\n" \
                    %(i, j, i, j))

    f.write ("\n")
    for i in range(FLAGS.propagator):
        f.write("REGISTER_DATAPATH(top.propagator%d.axon_storage): data/dram/dram%d.script\n" \
                %(i, i))
    
    f.write ("\n# Clock\n")
    f.write ("CLOCK_PERIOD(main): 50\n")
    f.write ("CLOCK_PERIOD(dram): 25\n")
    
    f.write ("\n# Parameters\n")
    f.write ("PARAMETER(num_neurons): %d\n" %(FLAGS.neurons))
    f.write ("PARAMETER(num_cores): %d\n" %(FLAGS.core))
    f.write ("PARAMETER(num_chips): %d\n" %(FLAGS.chip))
    f.write ("PARAMETER(num_propagators): %d\n" %(FLAGS.propagator))
    f.write ("PARAMETER(dram_size): %d\n" %(FLAGS.dram_size))
    f.write ("PARAMETER(max_timestep): %d\n" %(FLAGS.timestep))

    f.write ("\n# Unit power\n")


def main():
    print "chips: ", FLAGS.chip, "cores: ", FLAGS.core, "propagators", FLAGS.propagator
    
    spike_generator()
    meta_generator()

    spec_generator()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--neurons", 
            type=int, 
            default=20,
            help="The number of neurons per core")
    parser.add_argument("--timestep", 
            type=int, 
            default=5,
            help="Simulation timestep")
    parser.add_argument("--core",
            type=int,
            default=4,
            help="The number of neurocores")
    parser.add_argument("--chip",
            type=int,
            default=1,
            help="The number of neurochips")
    parser.add_argument("--propagator",
            type=int,
            default=1,
            help="The number of propagators")
    
    parser.add_argument("--spike_rate",
            type=int,
            default=1000,
            help="Spike firing rate")
    parser.add_argument("--distribution",
            type=str,
            default='uniform',
            help="Type of distribution (Not implemented")

    parser.add_argument("--sparsity",
            type=float,
            default=0.5,
            help="Neuron connection sparsity")

    parser.add_argument("--dram_size",
            type=int,
            default=1024,
            help="DRAM size")

    parser.add_argument("--path",
            type=str,
            default='./',
            help="Output file path")
    



    FLAGS, unparsed = parser.parse_known_args()

    main()
