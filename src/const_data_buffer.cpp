#include "const_data_buffer.h"

namespace voip
{

const_data_buffer::const_data_buffer(const void *ref_data
                                     , std::size_t ref_size)
    : m_ref_data(ref_data)
    , m_ref_size(ref_size)
{

}

void const_data_buffer::assign(const void *ref_data
                               , std::size_t ref_size)
{
    m_ref_data = ref_data;
    m_ref_size = ref_size;
}

const void *const_data_buffer::data() const
{
    return m_ref_data;
}

std::size_t const_data_buffer::size() const
{
    return m_ref_size;
}

void* const_data_buffer::map()
{
    return nullptr;
}


}
