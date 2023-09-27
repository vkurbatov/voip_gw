#include "opal_manager_config.h"
#include "opal_types.h"
#include "opal_utils.h"

namespace voip
{

opal_manager_config_t::opal_manager_config_t(const std::string_view& user_name
                                             , const std::string_view& display_name
                                             , std::uint32_t max_bitrate
                                             , std::uint16_t sip_port
                                             , std::uint16_t min_rtp_port
                                             , std::uint16_t max_rtp_port
                                             , std::uint16_t max_rtp_packet_size
                                             , const codec_array_t &audio_codecs
                                             , const codec_array_t &video_codecs)
    : call_manager_config_t(opal_type)
    , user_name(user_name)
    , display_name(display_name)
    , max_bitrate(max_bitrate)
    , sip_port(sip_port)
    , min_rtp_port(min_rtp_port)
    , max_rtp_port(max_rtp_port)
    , max_rtp_packet_size(max_rtp_packet_size)
    , audio_codecs(audio_codecs)
    , video_codecs(video_codecs)
{

}

bool opal_manager_config_t::operator ==(const call_manager_config_t &other) const
{
    return other.type == opal_type;
}

bool opal_manager_config_t::is_valid() const
{
    return type == opal_type;
}

}
