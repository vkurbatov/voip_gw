#ifndef VOIP_VIDEO_FRAME_INFO_H
#define VOIP_VIDEO_FRAME_INFO_H

#include <cstdint>

namespace voip
{

struct video_frame_info_t
{
    std::uint32_t   left;
    std::uint32_t   top;
    std::uint32_t   width;
    std::uint32_t   height;

    video_frame_info_t(std::uint32_t left = 0
                       , std::uint32_t top = 0
                       , std::uint32_t width = 0
                       , std::uint32_t height = 0);

    bool operator == (const video_frame_info_t& other) const;
    bool operator != (const video_frame_info_t& other) const;
};

}

#endif // VOIP_VIDEO_FRAME_INFO_H
