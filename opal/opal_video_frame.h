#ifndef VOIP_OPAL_VIDEO_FRAME_H
#define VOIP_OPAL_VIDEO_FRAME_H

#include "i_video_frame.h"

namespace voip
{

class opal_video_frame_ref : public i_video_frame
{
    i_data_buffer&      m_opal_video_buffer;
public:
    opal_video_frame_ref(i_data_buffer& buffer);

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

    // i_data_buffer interface
public:
    const void *data() const override;
    std::size_t size() const override;
    void *map() override;


    // i_media_frame interface
public:
    media_type_t type() const override;
    bool is_valid() const;
    const std::string &format() const override;
    void set_format(const std::string_view &format) override;
};

}

#endif // VOIP_OPAL_VIDEO_FRAME_H
