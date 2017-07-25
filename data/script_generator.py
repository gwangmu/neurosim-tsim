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
    pseudo_dram = {}

    dram_idx = (last_addr / FLAGS.dram_size) + 1

    for n in range(FLAGS.neurons):
        off_board = 0
        on_board = np.random.binomial(num_neurons, sparsity)
        length = off_board + on_board + FLAGS.delay # 1 for delay

        #length = (length // FLAGS.delay) * FLAGS.delay

        if ((last_addr + length) > dram_idx*FLAGS.dram_size):
            offset = (dram_idx * FLAGS.dram_size) - last_addr
            print ("(nidx: %d) last addr %d, offset %d, len %d" \
                    %(n, last_addr, offset, length))

            last_addr += offset
            for o in range(offset):
                dram_data.append(0)

            dram_idx += 1
            
            if(dram_idx > FLAGS.propagator):
                raise ValueError ("Small dram size")
       
        delay_len = []
        acc_len = 0
        for d in range(FLAGS.delay):
            d_len = float(length) * (float(d+1)/float(FLAGS.delay))
            d_len = int(d_len + 0.5) - acc_len
            acc_len += d_len

            delay_len.append(d_len)
        delay_len.append(0)

        meta_table.append((last_addr, delay_len[0]))
        for d in range(FLAGS.delay):
            d_len = delay_len[d]
            
            pseudo_data = (delay_len[d+1] << 32)  | (off_board << 16) | d_len
            pseudo_dram[last_addr] = pseudo_data

            if(not FLAGS.pseudo):
                dram_data += generate_syn (d_len) 

            last_addr += d_len

    return meta_table, last_addr, dram_data, pseudo_dram

     

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
            
            meta, last_addr, dram, pseudo_dram = generate_meta(last_addr)
            
            if(FLAGS.pseudo):
                dram_info.append (pseudo_dram)
            else:
                dram_info += dram

            for idx, m in enumerate(meta):
                addr, length = m

                d = (addr << 16) | (length)
                f.write("\taddr=0x%04x, data=0x%016x\n" %(idx, d))

            f.close()

    print "Required DRAM size", last_addr // FLAGS.propagator

    # dram_size = 1
    # while (dram_size < (len(dram_info) // FLAGS.propagator)):
    #     dram_size = dram_size * 2
    # FLAGS.dram_size = dram_size

    for i in range(FLAGS.propagator):
        f = open(FLAGS.path + 'data/dram/dram' + str(i) + '.script', 'w')
        fd = open(FLAGS.path + 'data/delay/delay' + str(i) + '.script', 'w')
        
        f.write("#!Tsim Script\n\n")
        f.write("CLASSNAME: DramFileRegister\n")
        f.write("DATA:\n")
        
        fd.write("#!Tsim Script\n\n")
        fd.write("CLASSNAME: DelayFileRegister\n")
        fd.write("DATA:\n")

        if(FLAGS.pseudo):
            is_end = False
            for it, addr_dict in enumerate(dram_info):
                if(is_end):
                    break

                addr_list = addr_dict.keys()
                addr_list.sort()

                for addr in addr_list:
                    if (addr < i*FLAGS.dram_size):
                        continue
                    elif (addr >= (i+1)*FLAGS.dram_size):
                        dram_info = dram_info[it:]
                        is_end = True
                        break
                    else:
                        dram_addr = addr - i*FLAGS.dram_size
                        f.write("\taddr=0x%04x, data=0x%016x\n" %(dram_addr, addr_dict[addr]))
        else:
            for addr in range(FLAGS.dram_size):
                idx = i*FLAGS.dram_size + addr
                if(idx < len(dram_info)):
                    f.write("\taddr=0x%04x, data=0x%016x\n" %(addr, dram_info[idx]))
                else:
                    f.write("\taddr=0x%04x, data=0x%016x\n" %(addr, 0))

        f.close() 
        fd.close()

def spec_generator(fast=0):
    f = open("simspec/neurosim" + str(fast)  + ".simspec", 'w')

    f.write ("#!Tsim Simulation Spec\n\n")

    spike_tgt = 'core_dyn_unit' if fast==1 else 'neuron_block'
    meta_tgt = 'core_dyn_unit' if fast==1 else 'axon_meta_table'

    f.write ("# External file\n")
    for i in range(FLAGS.chip):
        for j in range(FLAGS.core):
            f.write("FILESCRIPT_PATH(top.chip%d.core%d." %(i,j) \
                    + spike_tgt + \
                    "): data/spikes/spike%d_%d.script\n" %(i, j))

    f.write ("\n")
    for i in range(FLAGS.chip):
        for j in range(FLAGS.core):
            f.write("REGISTER_DATAPATH(top.chip%d.core%d." %(i, j) \
                    + meta_tgt + \
                    "): data/meta/meta%d_%d.script\n" %(i, j))

    f.write ("\n")
    for i in range(FLAGS.propagator):
        f.write("REGISTER_DATAPATH(top.propagator%d.axon_storage): data/dram/dram%d.script\n" \
                %(i, i))
        f.write("REGISTER_DATAPATH(top.propagator%d.delay_module.delay_storage): data/delay/delay%d.script\n" \
                %(i, i))
    
    f.write ("\n# Clock\n")
    f.write ("CLOCK_PERIOD(main): 4\n")
    f.write ("CLOCK_PERIOD(dram): 1\n")
    f.write ("CLOCK_PERIOD(ddr): 0.5\n")
    
    f.write ("\n# Parameters\n")
    f.write ("PARAMETER(num_neurons): %d\n" %(FLAGS.neurons))
    f.write ("PARAMETER(num_cores): %d\n" %(FLAGS.core))
    f.write ("PARAMETER(num_chips): %d\n" %(FLAGS.chip))
    f.write ("PARAMETER(num_propagators): %d\n" %(FLAGS.propagator))
    f.write ("PARAMETER(dram_size): %d\n" %(FLAGS.dram_size))
    f.write ("PARAMETER(max_timestep): %d\n" %(FLAGS.timestep))
    f.write ("PARAMETER(pseudo): %d\n" %(FLAGS.pseudo))
    f.write ("PARAMETER(fast): %d\n" %(fast))

    f.write ("\n# Unit power\n")


def main():
    print "chips: ", FLAGS.chip, "cores: ", FLAGS.core, "propagators", FLAGS.propagator
    
    spike_generator()
    meta_generator()

    spec_generator(0)
    spec_generator(1)

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

    parser.add_argument("--pseudo",
            type=bool, 
            default='True',
            help="Use pseudo DRAM")

    parser.add_argument("--delay",
            type=int,
            default=4,
            help="Maximum delay value")

    FLAGS, unparsed = parser.parse_known_args()

    main()
