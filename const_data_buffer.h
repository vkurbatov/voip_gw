#ifndef VOIP_CONST_DATA_BUFFER_H
#define VOIP_CONST_DATA_BUFFER_H

#include "i_data_buffer.h"

namespace voip
{

class const_data_buffer : public i_data_buffer
{
    const void*     m_ref_data;
    std::size_t     m_ref_size;

public:
    const_data_buffer(const void* ref_data
                      , std::size_t ref_size);

    void assign(const void* ref_data
                , std::size_t ref_size);

    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;
};

}

#endif // VOIP_CONST_DATA_BUFFER_H
