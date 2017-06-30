#include <Component/SRAMModule.h>

#include <Message/IndexMessage.h>
#include <Message/DeltaGMessage.h>
#include <Message/StateMessage.h>

#include <TSim/Utility/Prototype.h>
#include <TSim/Utility/Logging.h>

SRAMModule::SRAMModule (const char* clsname, string iname, Component* parent,
        uint32_t row_size, uint32_t col_size)
    : Module (clsname, iname, parent, 1)
{
    this->row_size_ = row_size;
    this->col_size_ = col_size;

    read_n = 0;
    write_n = 0;
}

