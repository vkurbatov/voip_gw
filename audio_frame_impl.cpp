#include "audio_frame_impl.h"

namespace voip
{


audio_frame_impl_basic::audio_frame_impl_basic(const audio_frame_info_t &info)
    : m_info(info)
{

}

media_type_t audio_frame_impl_basic::type() const
{
    return media_type_t::video;
}

bool audio_frame_impl_basic::is_valid() const
{
    return m_info.is_valid();
}

uint32_t audio_frame_impl_basic::sample_rate() const
{
    return m_info.sample_rate;
}

uint32_t audio_frame_impl_basic::channels() const
{
    return m_info.channels;
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

}
