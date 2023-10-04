#ifndef VOIP_MUTABLE_DATA_BUFFER_H
#define VOIP_MUTABLE_DATA_BUFFER_H

#include "i_data_buffer.h"

namespace voip
{

class mutable_data_buffer : public i_data_buffer
{
    void*           m_ref_data;
    std::size_t     m_ref_size;

public:
    mutable_data_buffer(void* ref_data
                      , std::size_t ref_size);

    void assign(void* ref_data
                , std::size_t ref_size);

    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;
};

}


#endif // VOIP_MUTABLE_DATA_BUFFER_H
