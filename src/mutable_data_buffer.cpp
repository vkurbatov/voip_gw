#include "mutable_data_buffer.h"

namespace voip
{

mutable_data_buffer::mutable_data_buffer(void *ref_data
                                     , std::size_t ref_size)
    : m_ref_data(ref_data)
    , m_ref_size(ref_size)
{

}

void mutable_data_buffer::assign(void *ref_data
                               , std::size_t ref_size)
{
    m_ref_data = ref_data;
    m_ref_size = ref_size;
}

const void *mutable_data_buffer::data() const
{
    return m_ref_data;
}

std::size_t mutable_data_buffer::size() const
{
    return m_ref_size;
}

void* mutable_data_buffer::map()
{
    return m_ref_data;
}


}
