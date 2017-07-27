#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>
#include <TSim/Pathway/IntegerMessage.h>

#include <Component/InputFeeder.h>
#include <Message/AxonMessage.h>
#include <Message/SignalMessage.h>
#include <Register/InputFileRegister.h>
#include <Register/InputRegisterWord.h>

#include <cinttypes>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

USING_TESTBENCH;

InputFeeder::InputFeeder (string iname, Component *parent,
        int num_propagators)
    : Module ("InputFeeder", iname, parent, 1), 
      num_propagators_ (num_propagators)
{
    // create ports
    for (int i=0; i<num_propagators_; i++)
    {
        PORT_axons.push_back(
                CreatePort ("axon" + to_string(i), Module::PORT_OUTPUT,
                    Prototype<AxonMessage>::Get()));
    }
    
    PORT_idle = CreatePort ("idle", Module::PORT_OUTPUT,
            Prototype<IntegerMessage>::Get());
    
    PORT_TSparity = CreatePort ("ts_parity", Module::PORT_INPUT,
            Prototype<IntegerMessage>::Get());

    is_idle_ = true;

    uint16_t spk_freq_ = GET_PARAMETER (spk_freq);
    uint32_t time_scale_ = GET_PARAMETER (time_scale);

    dram_size_ = GET_PARAMETER (dram_size);

    num_input_neurons_ = GET_PARAMETER (num_input_neurons);
    num_inputs_ = (num_input_neurons_ * 
                   (float(spk_freq_) / time_scale_) + 0.5);

    counter_ = num_inputs_;

    ts_parity_ = false;
    
    Register::Attr regattr (32, num_input_neurons_);
    SetRegister (new InputFileRegister (Register::DRAM, regattr));
}

// NOTE: called only if not stalled
void InputFeeder::Operation (Message **inmsgs, Message **outmsgs, 
        const uint32_t *outque_size, Instruction *instr)
{
    IntegerMessage *ts_msg =
        static_cast<IntegerMessage*>(inmsgs[PORT_TSparity]);
    if(ts_msg)
    {
        if(ts_msg->value != ts_parity_)
        {
            is_idle_ = false;
            counter_ = 0;
            ts_parity_ = ts_msg->value;

            INFO_PRINT ("[IF} Input Feed TS updated");
            outmsgs[PORT_idle] = new IntegerMessage (0);
        }
    }

    if(counter_ == num_inputs_)
    {
        if(!is_idle_)
        {
            is_idle_ = true;
            outmsgs[PORT_idle] = new IntegerMessage (1); 
        }
    }
    else
    {
        uint32_t read_addr = rand() % num_input_neurons_;

        const InputRegisterWord *word = 
            static_cast<const InputRegisterWord *>
                (GetRegister()->GetWord (read_addr));
        uint64_t metadata = word->value;
        
        uint16_t delay = (metadata >> 52) & 0x3ff;
        uint64_t dram_addr = (metadata >> 16) & 0xfffffffff;
        uint16_t len = metadata & 0xffff; 

        uint8_t prop_idx = dram_addr / dram_size_;
        uint64_t addr = dram_addr % dram_size_;

        INFO_PRINT ("[IF] Axon Message addr %lx, len %d", addr, len); 
        outmsgs[PORT_axons[prop_idx]] = new AxonMessage (0, addr, len, delay);
        
        counter_++;
    }
}
