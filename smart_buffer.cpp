#include "smart_buffer.h"

namespace voip
{

smart_buffer::smart_buffer(const void *data
                            , std::size_t size
                            , bool store)
    : m_ref_data(nullptr)
    , m_ref_size(0)
{
    assign(data
           , size
           , store);
}

smart_buffer::smart_buffer(raw_array_t &&raw_data)
{
    assign(std::move(raw_data));
}

void smart_buffer::assign(const void *data
                       , std::size_t size
                       , bool store)
{
    if (store)
    {
        m_ref_data = nullptr;
        m_ref_size = 0;
        if (data == nullptr)
        {
            m_store_data.assign(size, 0);
        }
        else
        {
            m_store_data.assign(static_cast<const std::uint8_t*>(data)
                                , static_cast<const std::uint8_t*>(data) + size);
        }
    }
    else
    {
        m_ref_data = data;
        m_ref_size = size;
        m_store_data.clear();
    }
}

void smart_buffer::assign(raw_array_t &&raw_data)
{
    m_ref_data = nullptr;
    m_ref_size = 0;
    m_store_data = std::move(raw_data);
}

raw_array_t smart_buffer::release()
{
    return std::move(m_store_data);
}

void smart_buffer::make_store()
{
    if (!is_stored())
    {
        assign(m_ref_data
               , m_ref_size
               , true);
    }
}

bool smart_buffer::is_stored() const
{
    return m_ref_data == nullptr;
}


void smart_buffer::clear()
{
    m_ref_data = nullptr;
    m_ref_size = 0;
    m_store_data.clear();
}

bool smart_buffer::is_empty() const
{
    return size() == 0;
}

void smart_buffer::resize(std::size_t new_size)
{
    make_store();
    m_store_data.resize(new_size);
}

const void *smart_buffer::data() const
{
    return is_stored()
            ? m_store_data.data()
            : m_ref_data;
}

std::size_t smart_buffer::size() const
{
    return is_stored()
            ? m_store_data.size()
            : m_ref_size;
}

void *smart_buffer::map()
{
    make_store();
    return m_store_data.data();
}

}
