#ifndef VOIP_I_DATA_BUFFER_H
#define VOIP_I_DATA_BUFFER_H

#include <cstdint>

namespace voip
{

class i_data_buffer
{
public:
    virtual ~i_data_buffer() = default;
    virtual const void* data() const = 0;
    virtual std::size_t size() const = 0;
    virtual void* map() = 0;
};

}

#endif // VOIP_I_DATA_BUFFER_H
