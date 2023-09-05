#ifndef VOIP_SMART_BUFFER_H
#define VOIP_SMART_BUFFER_H

#include "i_data_buffer.h"
#include <vector>

namespace voip
{

using raw_array_t = std::vector<std::uint8_t>;

class smart_buffer: public i_data_buffer
{
    const void*     m_ref_data;
    std::size_t     m_ref_size;
    raw_array_t     m_store_data;

public:
    smart_buffer(const void* data = nullptr
              , std::size_t size = 0
              , bool store = false);

    smart_buffer(raw_array_t&& raw_data);

    void assign(const void* data = nullptr
            , std::size_t size = 0
            , bool store = false);

    void assign(raw_array_t&& raw_data);

    raw_array_t release();
    void make_store();
    bool is_stored() const;
    void clear();
    bool is_empty() const;
    void resize(std::size_t new_size);

    // i_data_buffer inteface
public:
    const void* data() const override;
    std::size_t size() const override;
    void* map() override;
};

}

#endif // VOIP_SMART_BUFFER_H
