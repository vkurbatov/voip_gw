#include "opal_utils.h"

#include <opal/opal/transcoders.h>

namespace voip
{

media_type_t get_media_type(const std::string &opal_media_type)
{
    if (opal_media_type == OpalMediaType::Audio())
    {
        return media_type_t::audio;
    }
    else if (opal_media_type == OpalMediaType::Video())
    {
        return media_type_t::video;
    }
    return media_type_t::undefined;
}

codec_info_t::array_t get_supported_opal_codecs(media_type_t media_type)
{
    codec_info_t::array_t codecs;
    auto all_formats = OpalMediaFormat::GetAllRegisteredMediaFormats();
    auto possible_formats = OpalTranscoder::GetPossibleFormats(all_formats);

    for (const auto& f : possible_formats)
    {
        auto opal_media_type = get_media_type(f.GetMediaType());
        if (media_type == media_type_t::undefined
                || opal_media_type == media_type)
        {
            if (f.IsTransportable())
            {
                if (auto encode_name = f.GetEncodingName())
                {
                    codecs.emplace_back(opal_media_type
                                        , encode_name
                                        , std::string(f.GetName())
                                        , std::string(f.GetDescription())
                                        , f.GetClockRate());
                }
            }
        }
    }

    return codecs;
}


}
