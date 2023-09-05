#ifndef VOIP_MEDIA_TYPES_H
#define VOIP_MEDIA_TYPES_H

#include <cstdint>

namespace voip
{

enum class content_role_t
{
    undefined,
    slides,
    speaker,
    sl,
    main,
    alt
};

enum class media_type_t
{
    undefined,
    audio,
    video,
    application
};

enum class stream_type_t
{
    undefined,
    inbound,
    outbound
};


}

#endif // VOIP_MEDIA_TYPES_H
