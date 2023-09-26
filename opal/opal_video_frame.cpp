#include "opal_video_frame.h"
#include "video_frame_impl.h"
#include "opal/codec/opalplugin.h"

namespace voip
{

std::size_t opal_video_frame_ref::opal_video_header_size()
{
    return sizeof(PluginCodec_Video_FrameHeader);
}

opal_video_frame_ref::opal_video_frame_ref(i_data_buffer &buffer)
    : m_opal_video_buffer(buffer)
{

}

uint32_t opal_video_frame_ref::left() const
{
    return static_cast<const PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.data())->x;
}

uint32_t opal_video_frame_ref::top() const
{
    return static_cast<const PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.data())->y;
}

uint32_t opal_video_frame_ref::width() const
{
    return static_cast<const PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.data())->width;
}

uint32_t opal_video_frame_ref::height() const
{
    return static_cast<const PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.data())->height;
}

void opal_video_frame_ref::set_left(uint32_t left)
{
    if (auto header = static_cast<PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.map()))
    {
        header->x = left;
    }
}

void opal_video_frame_ref::set_top(uint32_t top)
{
    if (auto header = static_cast<PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.map()))
    {
        header->y = top;
    }
}

void opal_video_frame_ref::set_width(uint32_t width)
{
    if (auto header = static_cast<PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.map()))
    {
        header->width = width;
    }
}

void opal_video_frame_ref::set_height(uint32_t height)
{
    if (auto header = static_cast<PluginCodec_Video_FrameHeader*>(m_opal_video_buffer.map()))
    {
        header->height = height;
    }
}

const void *opal_video_frame_ref::data() const
{
    return static_cast<const std::uint8_t*>(m_opal_video_buffer.data())
                                            + sizeof(PluginCodec_Video_FrameHeader);
}

std::size_t opal_video_frame_ref::size() const
{
    return m_opal_video_buffer.size() - sizeof(PluginCodec_Video_FrameHeader);
}

void *opal_video_frame_ref::map()
{
    return static_cast<std::uint8_t*>(m_opal_video_buffer.map())
                                       + sizeof(PluginCodec_Video_FrameHeader);
}

media_type_t opal_video_frame_ref::type() const
{
    return media_type_t::video;
}

bool opal_video_frame_ref::is_valid() const
{
    return m_opal_video_buffer.size() >= sizeof(PluginCodec_Video_FrameHeader);
}

const std::string &opal_video_frame_ref::format() const
{
    static const std::string single_format = "YUV420";
    return single_format;
}

void opal_video_frame_ref::set_format(const std::string_view& /*format*/)
{
    // not impl
}

i_media_frame::u_ptr_t opal_video_frame_ref::clone() const
{
    if (is_valid())
    {
        return video_frame_impl::create(smart_buffer(data()
                                              , size()
                                              , true)
                                        , { format(), left(), top(), width(), height() });
    }

    return nullptr;
}


}
