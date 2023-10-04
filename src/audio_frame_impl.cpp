#include "audio_frame_impl.h"

namespace voip
{


audio_frame_impl_basic::audio_frame_impl_basic(const audio_frame_info_t &info)
    : m_info(info)
{

}

void audio_frame_impl_basic::set_info(const audio_frame_info_t &info)
{
    m_info = info;
}

const audio_frame_info_t &audio_frame_impl_basic::frame_info() const
{
    return m_info;
}

media_type_t audio_frame_impl_basic::type() const
{
    return media_type_t::audio;
}

bool audio_frame_impl_basic::is_valid() const
{
    return m_info.is_valid();
}

const std::string &audio_frame_impl_basic::format() const
{
    return m_info.format;
}

uint32_t audio_frame_impl_basic::sample_rate() const
{
    return m_info.sample_rate;
}

uint32_t audio_frame_impl_basic::channels() const
{
    return m_info.channels;
}

void audio_frame_impl_basic::set_format(const std::string_view &format)
{
    m_info.format = format;
}

void audio_frame_impl_basic::set_sample_rate(uint32_t sample_rate)
{
    m_info.sample_rate = sample_rate;
}

void audio_frame_impl_basic::set_channels(uint32_t channels)
{
    m_info.channels = channels;
}

audio_frame_impl_ref::audio_frame_impl_ref(i_data_buffer &audio_buffer
                                           , const audio_frame_info_t &info)
    : audio_frame_impl_basic(info)
    , m_audio_buffer(audio_buffer)
{

}

const void *audio_frame_impl_ref::data() const
{
    return m_audio_buffer.data();
}

std::size_t audio_frame_impl_ref::size() const
{
    return m_audio_buffer.size();
}

void *audio_frame_impl_ref::map()
{
    return m_audio_buffer.map();
}

i_media_frame::u_ptr_t audio_frame_impl_ref::clone() const
{
    return audio_frame_impl::create(smart_buffer(m_audio_buffer.data()
                                                 , m_audio_buffer.size()
                                                 , true)
                                    , m_info);
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(const smart_buffer &audio_buffer
                                                   , const audio_frame_info_t &info)
{
    return std::make_unique<audio_frame_impl>(audio_buffer
                                              , info);
}

audio_frame_impl::u_ptr_t audio_frame_impl::create(smart_buffer &&audio_buffer
                                                   , const audio_frame_info_t &info)
{
    return std::make_unique<audio_frame_impl>(std::move(audio_buffer)
                                              , info);
}

audio_frame_impl::audio_frame_impl(const smart_buffer &audio_buffer
                                   , const audio_frame_info_t &info)
    : audio_frame_impl_basic(info)
    , m_audio_buffer(audio_buffer)
{

}

audio_frame_impl::audio_frame_impl(smart_buffer &&audio_buffer
                                   , const audio_frame_info_t &info)
    : audio_frame_impl_basic(info)
    , m_audio_buffer(std::move(audio_buffer))
{

}

void audio_frame_impl::set_buffer(smart_buffer &&audio_buffer)
{
    m_audio_buffer = std::move(audio_buffer);
}

const void *audio_frame_impl::data() const
{
    return m_audio_buffer.data();
}

std::size_t audio_frame_impl::size() const
{
    return m_audio_buffer.size();
}

void *audio_frame_impl::map()
{
    return m_audio_buffer.map();
}

i_media_frame::u_ptr_t audio_frame_impl::clone() const
{
    return create(m_audio_buffer.fork()
                  , m_info);
}

}
