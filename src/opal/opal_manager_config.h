#ifndef VOIP_OPAL_MANAGER_CONFIG_H
#define VOIP_OPAL_MANAGER_CONFIG_H

#include "call_manager_config.h"
#include <string>
#include <vector>

namespace voip
{

struct opal_manager_config_t : public call_manager_config_t
{
    using codec_array_t = std::vector<std::string>;
    std::string                 user_name;
    std::string                 display_name;
    std::uint32_t               max_bitrate;
    std::uint16_t               sip_port;
    std::uint16_t               min_rtp_port;
    std::uint16_t               max_rtp_port;
    std::uint16_t               max_rtp_packet_size;

    codec_array_t               audio_codecs;
    codec_array_t               video_codecs;


    opal_manager_config_t(const std::string_view& user_name = {}
                          , const std::string_view& display_name = {}
                          , std::uint32_t max_bitrate = 0
                          , std::uint16_t sip_port = 0
                          , std::uint16_t min_rtp_port = 0
                          , std::uint16_t max_rtp_port = 0
                          , std::uint16_t max_rtp_packet_size = 0
                          , const codec_array_t& audio_codecs = {}
                          , const codec_array_t& video_codecs = {});

    // call_manager_config_t interface
public:
    bool operator ==(const call_manager_config_t &other) const override;
    bool is_valid() const override;
};

}

#endif // VOIP_OPAL_MANAGER_CONFIG_H
