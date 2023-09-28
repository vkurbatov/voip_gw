#ifndef VOIP_API_H
#define VOIP_API_H

#include <stdint.h>

#define __extern_c extern "C"

#define __long_string_size  255
#define __short_string_size 40

typedef int32_t vgw_handle_t;
typedef int32_t vgw_result_t;

#define vgw_ok (0)
#define vgw_failed (-1)

#define __vgw_is_result_ok(result) ((result) >= 0)
#define __vgw_is_result_failed(result) ((result) < 0)

#pragma pack(push, 1)

enum vgw_message_type_t
{
    message_manager = 0,
    message_call
};

enum vgw_media_type_t
{
    undefined = 0,
    audio,
    video,
    application
};

enum vgw_stream_type_t
{
    inbound = 0,
    outbound
};

struct vgw_call_manager_config_t
{
    const char*                 user_name;
    const char*                 display_name;
    uint32_t                    max_bitrate;
    uint16_t                    min_rtp_port;
    uint16_t                    max_rtp_port;
    uint16_t                    max_rtp_packet_size;
    const char*                 audio_codecs;
    const char*                 video_codecs;
};

struct vgw_stream_info_t
{
    vgw_media_type_t            media_type;
    vgw_stream_type_t           stream_type;
    int32_t                     stream_id;
};

struct vgw_call_info_t
{
    vgw_handle_t                call_handle;
    const char*                 remote_url;
    const char*                 call_id;
};

enum vgw_sync_mode_t
{
    sync,
    async
};

enum vgw_manager_event_type_t
{
    started = 0,
    stopped,
    create_call,
    release_call
};

struct vgw_manager_message_t
{
    vgw_manager_event_type_t    event_type;
    vgw_handle_t                handle;
    union
    {
        vgw_call_info_t         call_info; // create_call, release_call
    }                           body;
};

struct vgw_audio_info_t
{
    const char*                 format;
    uint32_t                    sample_rate;
    uint32_t                    channels;
};

struct vgw_video_info_t
{
    const char*                 format;
    uint32_t                    width;
    uint32_t                    height;
};

struct vgw_media_frame_t
{
    int32_t                     stream_id;
    vgw_media_type_t            media_type;

    union
    {
        vgw_audio_info_t        audio_info;
        vgw_video_info_t        video_info;
    }                           info;

    uint32_t                    buffer_size;
    void*                       buffer_ptr;
};

enum vgw_call_event_type_t
{
    open_stream = 0,
    close_stream,
    read_frame,
    write_frame,
    user_input
};

struct vgw_call_message_t
{
    vgw_call_event_type_t       event_type;
    vgw_handle_t                handle;
    union
    {
        vgw_stream_info_t       stream_info;    // open_stream, close_stream
        vgw_media_frame_t       media_frame;    // read_frame, write_frame
        const char*             tones;          // user_input
    }                           body;
};

#pragma pack(pop)

__extern_c typedef vgw_result_t (*message_callback_t)(vgw_message_type_t type
                                                      , void* message);

__extern_c vgw_handle_t create_manager(const vgw_call_manager_config_t* manager_config
                                       , message_callback_t* callback_manager);
__extern_c vgw_result_t release_manager(vgw_handle_t manager_handle);
__extern_c vgw_result_t start_manager(vgw_handle_t manager_handle);
__extern_c vgw_result_t stop_manager(vgw_handle_t manager_handle);
__extern_c vgw_result_t make_call(vgw_handle_t manager_handle
                                  , const char* url);

__extern_c vgw_result_t send_user_input(vgw_handle_t call_handle
                                        , const char* tones);
__extern_c vgw_result_t hangup_call(vgw_handle_t call_handle);


#endif // VOIP_API_H
