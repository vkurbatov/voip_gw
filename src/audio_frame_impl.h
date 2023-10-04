#ifndef VOIP_AUDIO_FRAME_IMPL_H
#define VOIP_AUDIO_FRAME_IMPL_H

#include "i_audio_frame.h"
#include "audio_frame_info.h"
#include "smart_buffer.h"

namespace voip
{

class audio_frame_impl_basic : public i_audio_frame
{
protected:
    audio_frame_info_t  m_info;
public:
    audio_frame_impl_basic(const audio_frame_info_t& info = {});

    void set_info(const audio_frame_info_t& info);
    const audio_frame_info_t& frame_info() const;

    // i_media_frame interface
public:
    media_type_t type() const override;
    bool is_valid() const override;
    const std::string& format() const override;
    void set_format(const std::string_view& format) override;

    // i_audio_frame interface
public:
    uint32_t sample_rate() const override;
    uint32_t channels() const override;

    void set_sample_rate(uint32_t sample_rate) override;
    void set_channels(uint32_t channels) override;
};

class audio_frame_impl_ref : public audio_frame_impl_basic
{
    i_data_buffer&      m_audio_buffer;

public:
    audio_frame_impl_ref(i_data_buffer& audio_buffer
                         , const audio_frame_info_t& info = {});


    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;

    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;
};

class audio_frame_impl : public audio_frame_impl_basic
{
    smart_buffer        m_audio_buffer;

public:

    using u_ptr_t = std::unique_ptr<audio_frame_impl>;

    static u_ptr_t create(const smart_buffer& audio_buffer
                          , const audio_frame_info_t& info);

    static u_ptr_t create(smart_buffer&& audio_buffer = {}
                          , const audio_frame_info_t& info = {});

    audio_frame_impl(const smart_buffer& audio_buffer
                     , const audio_frame_info_t& info);
    audio_frame_impl(smart_buffer&& audio_buffer = {}
                     , const audio_frame_info_t& info = {});

    void set_buffer(smart_buffer&& audio_buffer);

    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;

    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;
};

}

#endif // VOIP_AUDIO_FRAME_IMPL_H
