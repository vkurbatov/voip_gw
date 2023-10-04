#ifndef VOIP_CODEC_INFO_H
#define VOIP_CODEC_INFO_H

#include "media_types.h"
#include <string>
#include <vector>

namespace voip
{

struct codec_info_t
{
    using array_t = std::vector<codec_info_t>;

    media_type_t    type;
    std::string     name;
    std::string     full_name;
    std::string     description;
    std::uint32_t   sample_rate;

    codec_info_t(media_type_t type = media_type_t::undefined
                , const std::string_view& name = {}
                , const std::string_view& full_name = {}
                , const std::string_view& description = {}
                , std::uint32_t sample_rate = 0);
};

}

#endif // VOIP_CODEC_INFO_H
