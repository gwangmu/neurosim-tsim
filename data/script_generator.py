import argparse
import random
import numpy as np

FLAGS = None

def filename (pre, chip, core):
    return pre + str(chip) + '_' + str(core) + '.script'

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

def generate_meta(last_addr):
    num_neurons = FLAGS.neurons * FLAGS.core * FLAGS.chip
    sparsity = FLAGS.sparsity
    
    meta_table = [] 
    for n in range(FLAGS.neurons):
        length = np.random.binomial(num_neurons, sparsity)
        meta_table.append((last_addr, length))

        last_addr += length

    return meta_table, last_addr

     

def meta_generator():
    random.seed(946)

    num_neurons = FLAGS.neurons * FLAGS.core * FLAGS.chip

    last_addr = 0
    for i in range (FLAGS.chip):
        for j in range (FLAGS.core):
            f = open(FLAGS.path + filename('data/meta/meta',i,j), 'w')

            f.write("#!Tsim Script\n\n")
            f.write("CLASSNAME: MetaRegister\n")
            f.write("Data:\n")
            
            meta, last_addr = generate_meta(last_addr)
            for idx, m in enumerate(meta):
                addr, length = m

                d = (addr << 16) | (length)
                f.write("\taddr=0x%04x, data=0x%016x\n" %(idx, d))

            f.close()

def spec_generator():
    f = open("simspec/neurosim.simspec", 'w')

    f.write ("#!Tsim Simulation Spec\n\n")

    f.write ("# External file\n")
    for i in range(FLAGS.chip):
        for j in range(FLAGS.core):
            f.write("FILESCRIPT_PATH(top.chip%d.core%d.neuron_block): data/spikes/spike%d_%d.script\n" \
                    %(i, j, i, j))

    f.write ("\n# Clock\n")
    f.write ("CLOCK_PERIOD(main): 500\n\n")

    f.write ("# Unit power\n")


def main():
    print "chips: ", FLAGS.chip, "cores: ", FLAGS.core
    
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


    parser.add_argument("--path",
            type=str,
            default='./',
            help="Output file path")
    



    FLAGS, unparsed = parser.parse_known_args()

    main()
