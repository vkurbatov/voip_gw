#ifndef VOIP_OPAL_UTILS_H
#define VOIP_OPAL_UTILS_H

#include "codec_info.h"

namespace voip
{

media_type_t get_media_type(const std::string& opal_media_type);

codec_info_t::array_t get_supported_opal_codecs(media_type_t media_type);

}

#endif // VOIP_OPAL_UTILS_H
