#ifndef VOIP_VIDEO_FRAME_IMPL_H
#define VOIP_VIDEO_FRAME_IMPL_H

#include "i_video_frame.h"
#include "video_frame_info.h"
#include "smart_buffer.h"

namespace voip
{

class video_frame_impl_basic : public i_video_frame
{
protected:
    video_frame_info_t  m_info;
public:
    video_frame_impl_basic(const video_frame_info_t& info = {});

    void set_frame_info(const video_frame_info_t& info);
    const video_frame_info_t& frame_info() const;
    // i_media_frame interface
public:
    media_type_t type() const override;
    bool is_valid() const override;
    const std::string &format() const override;
    void set_format(const std::string_view &format) override;

    // i_video_frame interface
public:
    uint32_t left() const override;
    uint32_t top() const override;
    uint32_t width() const override;
    uint32_t height() const override;
    void set_left(uint32_t left) override;
    void set_top(uint32_t top) override;
    void set_width(uint32_t width) override;
    void set_height(uint32_t height) override;
};

class video_frame_impl_ref : public video_frame_impl_basic
{
    i_data_buffer&      m_video_buffer;

public:
    video_frame_impl_ref(i_data_buffer& video_buffer
                         , const video_frame_info_t& info = {});

    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;

    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;
};

class video_frame_impl : public video_frame_impl_basic
{
    smart_buffer        m_video_buffer;

public:

    using u_ptr_t = std::unique_ptr<video_frame_impl>;

    static u_ptr_t create(const smart_buffer& video_buffer
                          , const video_frame_info_t& info);

    static u_ptr_t create(smart_buffer&& video_buffer = {}
                          , const video_frame_info_t& info = {});

    video_frame_impl(const smart_buffer& video_buffer
                     , const video_frame_info_t& info);
    video_frame_impl(smart_buffer&& video_buffer = {}
                     , const video_frame_info_t& info = {});

    void set_buffer(smart_buffer&& video_buffer);

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

#endif // VOIP_VIDEO_FRAME_IMPL_H
