#include "video_frame_impl.h"

namespace voip
{

video_frame_impl_basic::video_frame_impl_basic(const video_frame_info_t &info)
    : m_info(info)
{

}

void video_frame_impl_basic::set_frame_info(const video_frame_info_t &info)
{
    m_info = info;
}

media_type_t video_frame_impl_basic::type() const
{
    return media_type_t::video;
}

bool video_frame_impl_basic::is_valid() const
{
    return m_info.is_valid()
            && data() != nullptr;
}

const std::string &video_frame_impl_basic::format() const
{
    return m_info.format;
}

void video_frame_impl_basic::set_format(const std::string_view &format)
{
    m_info.format = format;
}

uint32_t video_frame_impl_basic::left() const
{
    return m_info.left;
}

uint32_t video_frame_impl_basic::top() const
{
    return m_info.top;
}

uint32_t video_frame_impl_basic::width() const
{
    return m_info.width;
}

uint32_t video_frame_impl_basic::height() const
{
    return m_info.height;
}

void video_frame_impl_basic::set_left(uint32_t left)
{
    m_info.left = left;
}

void video_frame_impl_basic::set_top(uint32_t top)
{
    m_info.top = top;
}

void video_frame_impl_basic::set_width(uint32_t width)
{
    m_info.width = width;
}

void video_frame_impl_basic::set_height(uint32_t height)
{
    m_info.height = height;
}

video_frame_impl_ref::video_frame_impl_ref(i_data_buffer &video_buffer
                                           , const video_frame_info_t &info)
    : video_frame_impl_basic(info)
    , m_video_buffer(video_buffer)
{

}

const void *video_frame_impl_ref::data() const
{
    return m_video_buffer.data();
}

std::size_t video_frame_impl_ref::size() const
{
    return m_video_buffer.size();
}

void *video_frame_impl_ref::map()
{
    return m_video_buffer.map();
}

video_frame_impl::video_frame_impl(const smart_buffer &video_buffer
                                   , const video_frame_info_t &info)
    : video_frame_impl_basic(info)
    , m_video_buffer(video_buffer)
{

}

video_frame_impl::video_frame_impl(smart_buffer &&video_buffer
                                   , const video_frame_info_t &info)
    : video_frame_impl_basic(info)
    , m_video_buffer(std::move(video_buffer))
{

}

void video_frame_impl::set_buffer(smart_buffer &&video_buffer)
{
    m_video_buffer = std::move(video_buffer);
}

const void *video_frame_impl::data() const
{
    return m_video_buffer.data();
}

std::size_t video_frame_impl::size() const
{
    return m_video_buffer.size();
}

void *video_frame_impl::map()
{
    return m_video_buffer.map();
}

}
