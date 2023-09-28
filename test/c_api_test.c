#include "c_api_test.h"
#include "c_api/voip_api.h"

#include <unistd.h>
#include <stdio.h>

vgw_result_t callback(enum vgw_message_type_t type
                             , void* message)
{
    // printf("OnMessage: %d", type);
    return vgw_failed;
}

int app(int argc, const char *argv[])
{
    struct vgw_call_manager_config_t manager_config;
    manager_config.user_name = "user";
    manager_config.display_name = "user";
    manager_config.max_bitrate = 2048000;
    manager_config.min_rtp_port = 10000;
    manager_config.max_rtp_port = 11000;
    manager_config.max_rtp_packet_size = 1400;
    manager_config.audio_codecs = "G7221;PCMA;PCMU";
    manager_config.video_codecs = "H264;H265;H263-1998";

    vgw_handle_t manager = create_manager(&manager_config
                                          , callback);

    usleep(1000000);

    start_manager(manager);

    usleep(1000000);

    stop_manager(manager);

    usleep(1000000);

    release_manager(manager);

    usleep(1000000000);

    return 0;
}
