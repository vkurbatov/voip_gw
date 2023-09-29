#include "c_api_test.h"
#include "c_api/voip_api.h"

#include <unistd.h>
#include <stdio.h>

uint32_t stop = 0;
vgw_handle_t call_handle = vgw_failed;
vgw_handle_t audio_queue = vgw_failed;
vgw_handle_t video_queue = vgw_failed;

static vgw_result_t on_manager_started(vgw_handle_t handle)
{
    return vgw_ok;
}

static vgw_result_t on_manager_stopped(vgw_handle_t handle)
{
    return vgw_ok;
}

static vgw_result_t on_manager_create_call(vgw_handle_t handle
                                           , struct vgw_call_info_t* call_info)
{
    if (__vgw_is_result_failed(call_handle))
    {
        call_handle = handle;
        return vgw_ok;
    }

    return vgw_failed;
}

static vgw_result_t on_manager_release_call(vgw_handle_t handle
                                            , struct vgw_call_info_t* call_info)
{
    if (call_handle == handle)
    {
        call_handle = vgw_failed;
        stop = 1;
        return vgw_ok;
    }
    return vgw_failed;
}

inline static vgw_result_t on_manager_message(struct vgw_manager_message_t* manager_message)
{
    switch(manager_message->event_type)
    {
        case vgw_manager_event_started:
            return on_manager_started(manager_message->handle);
        break;
        case vgw_manager_event_stopped:
            return on_manager_stopped(manager_message->handle);
        break;
        case vgw_manager_event_create_call:
            return on_manager_create_call(manager_message->handle, &manager_message->body.call_info);
        break;
        case vgw_manager_event_release_call:
            return on_manager_release_call(manager_message->handle, &manager_message->body.call_info);
        break;
        default:;
    }

    return vgw_failed;
}

static vgw_result_t on_call_open_stream(vgw_handle_t handle
                                        , struct vgw_stream_info_t* stream_info)
{
    return vgw_ok;
}

static vgw_result_t on_call_close_stream(vgw_handle_t handle
                                         , struct vgw_stream_info_t* stream_info)
{
    return vgw_ok;
}

static vgw_result_t on_call_read_frame(vgw_handle_t handle
                                       , struct vgw_media_frame_t* media_frame)
{
    if (call_handle == handle)
    {
        switch(media_frame->media_type)
        {
            case vgw_media_type_audio:
            {
                if (__vgw_is_result_ok(audio_queue))
                {
                    return vgw_pop_frame(audio_queue
                                         , media_frame
                                         , 100);
                }
            }
            break;
            case vgw_media_type_video:
            {
                if (__vgw_is_result_ok(video_queue))
                {
                    return vgw_pop_frame(video_queue
                                         , media_frame
                                         , 100);
                }
            }
            break;
            default:;
        }
    }

    return vgw_failed;
}

static vgw_result_t on_call_write_frame(vgw_handle_t handle
                                        , struct vgw_media_frame_t* media_frame)
{
    if (call_handle == handle)
    {
        switch(media_frame->media_type)
        {
            case vgw_media_type_audio:
            {
                if (__vgw_is_result_ok(audio_queue))
                {
                    return vgw_push_frame(audio_queue
                                         , media_frame);
                }
            }
            break;
            case vgw_media_type_video:
            {
                if (__vgw_is_result_ok(video_queue))
                {
                    return vgw_push_frame(video_queue
                                          , media_frame);
                }
            }
            break;
            default:;
        }
    }

    return vgw_failed;
}

static vgw_result_t on_call_user_input(vgw_handle_t handle
                                       , const char* tones)
{
    if (call_handle == handle)
    {
        vgw_send_user_input(handle
                            , tones);
        return vgw_ok;
    }

    return vgw_failed;
}

static vgw_result_t on_call_message(struct vgw_call_message_t* call_message)
{
    switch(call_message->event_type)
    {
        case vgw_call_event_open_stream:
            return on_call_open_stream(call_message->handle, &call_message->body.stream_info);
        break;
        case vgw_call_event_close_stream:
            return on_call_close_stream(call_message->handle, &call_message->body.stream_info);
        break;
        case vgw_call_event_read_frame:
            return on_call_read_frame(call_message->handle, &call_message->body.media_frame);
        break;
        case vgw_call_event_write_frame:
            return on_call_write_frame(call_message->handle, &call_message->body.media_frame);
        break;
        case vgw_call_event_user_input:
            return on_call_user_input(call_message->handle, call_message->body.tones);
        break;
        default:;
    }

    return vgw_failed;
}

static vgw_result_t callback(enum vgw_message_type_t type
                             , void* message)
{
    switch(type)
    {
        case vgw_message_type_manager:
            return on_manager_message((struct vgw_manager_message_t*)message);
        break;
        case vgw_message_type_call:
            return on_call_message((struct vgw_call_message_t*)message);
        break;
        default:;
    }

    return vgw_failed;
}

int c_app(int argc
          , const char *argv[])
{
    struct vgw_call_manager_config_t manager_config = {};
    manager_config.user_name = "vasiliy";
    manager_config.display_name = "kurbatov";
    manager_config.max_bitrate = 2048000;
    manager_config.min_rtp_port = 10000;
    manager_config.max_rtp_port = 11000;
    manager_config.max_rtp_packet_size = 1400;
    manager_config.audio_codecs = "G7221;PCMA;PCMU";
    manager_config.video_codecs = "H264;H265;H263-1998";

    vgw_handle_t manager = vgw_create_manager(&manager_config
                                              , callback);

    if (__vgw_is_result_ok(manager))
    {
        audio_queue = vgw_create_frame_queue(5);
        video_queue = vgw_create_frame_queue(5);

        if (__vgw_is_result_ok(vgw_start_manager(manager)))
        {

            if (argc == 2)
            {
                vgw_make_call(manager
                              , argv[1]);
            }

            while(!stop)
            {
                usleep(100000);
            }

            vgw_stop_manager(manager);
        }

        vgw_release_manager(manager);
    }

    return 0;
}
