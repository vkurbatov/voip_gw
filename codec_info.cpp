#include "codec_info.h"

namespace voip
{

codec_info_t::codec_info_t(media_type_t type
                           , const std::string_view &name
                           , const std::string_view &full_name
                           , const std::string_view &description
                           , uint32_t sample_rate)
    : type(type)
    , name(name)
    , full_name(full_name)
    , description(description)
    , sample_rate(sample_rate)
{

}

}
