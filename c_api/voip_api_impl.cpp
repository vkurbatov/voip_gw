#include "voip_api.h"

#include "i_engine.h"
#include "i_media_session.h"
#include "i_media_stream.h"
#include "i_media_format.h"
#include "i_media_frame.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"

#include "voip_control.h"

#include "utils/common_utils.h"

#include "opal/opal_manager_config.h"

#include <unordered_map>
#include <mutex>

namespace voip
{

namespace detail
{

opal_manager_config_t create_opal_manager_config(const vgw_call_manager_config_t& c_config)
{
    opal_manager_config_t opal_config;

    if (c_config.user_name)
    {
        opal_config.user_name = c_config.user_name;
    }

    if (c_config.display_name)
    {
        opal_config.display_name = c_config.display_name;
    }

    opal_config.max_bitrate = c_config.max_bitrate;
    opal_config.min_rtp_port = c_config.min_rtp_port;
    opal_config.max_rtp_port = c_config.max_rtp_port;
    opal_config.max_rtp_packet_size = c_config.max_rtp_packet_size;

    if (c_config.audio_codecs)
    {
        opal_config.audio_codecs = utils::split_string(c_config.audio_codecs, ";");
    }

    if (c_config.video_codecs)
    {
        opal_config.video_codecs = utils::split_string(c_config.video_codecs, ";");
    }

    return opal_config;
}

void fill_c_frame(const i_media_frame& frame
                  , const i_media_stream& stream
                  , vgw_media_frame_t& c_frame)
{
    c_frame.stream_id = stream.session().id();

    switch(frame.type())
    {
        case media_type_t::audio:
        {
            auto& audio_frame = static_cast<const i_audio_frame&>(frame);
            c_frame.media_type = audio;
            c_frame.info.audio_info.format = audio_frame.format().c_str();
            c_frame.info.audio_info.sample_rate = audio_frame.sample_rate();
            c_frame.info.audio_info.channels = audio_frame.channels();
        }
        break;
        case media_type_t::video:
        {
            auto& video_frame = static_cast<const i_video_frame&>(frame);
            c_frame.media_type = video;
            c_frame.info.video_info.format = video_frame.format().c_str();
            c_frame.info.video_info.width = video_frame.width();
            c_frame.info.video_info.height = video_frame.height();
        }
        break;
        default:;
    }

    c_frame.buffer_ptr = const_cast<void*>(frame.data());
    c_frame.buffer_size = frame.size();
}

void merge_frame_info(i_media_frame& frame
                      , const vgw_media_frame_t& c_frame)
{
    switch(frame.type())
    {
        case media_type_t::audio:
        {
            auto& audio_frame = static_cast<i_audio_frame&>(frame);
            if (c_frame.info.audio_info.format)
            {
                audio_frame.set_format(c_frame.info.audio_info.format);
            }

            audio_frame.set_sample_rate(c_frame.info.audio_info.sample_rate);
            audio_frame.set_channels(c_frame.info.audio_info.channels);
        }
        break;
        case media_type_t::video:
        {
            auto& video_frame = static_cast<i_video_frame&>(frame);
            if (c_frame.info.video_info.format)
            {
                video_frame.set_format(c_frame.info.video_info.format);
            }

            video_frame.set_width(c_frame.info.video_info.width);
            video_frame.set_height(c_frame.info.video_info.height);
        }
        break;
        default:;
    }
}

}

class api_adapter: public i_call_manager::i_listener
        , public i_call::i_listener
        , public i_media_session::i_listener
{

    using mutex_t = std::mutex;

    class call_container
    {
        using call_map_t = std::unordered_map<vgw_handle_t, i_call*>;
        mutable mutex_t             m_mutex;
        call_map_t                  m_calls;

    public:
        call_container()
        {

        }

        bool add_call(i_call* call)
        {
            std::lock_guard lock(m_mutex);
            return m_calls.emplace(call->index()
                                    , call).second;
        }

        bool remove_call(i_call* call)
        {
            std::lock_guard lock(m_mutex);
            return m_calls.erase(call->index()) > 0;
        }

        i_call* get_call(vgw_handle_t handle)
        {
            std::lock_guard lock(m_mutex);
            if (auto it = m_calls.find(handle); it != m_calls.end())
            {
                return it->second;
            }

            return nullptr;
        }

        const i_call* get_call(vgw_handle_t handle) const
        {
            std::lock_guard lock(m_mutex);
            if (auto it = m_calls.find(handle); it != m_calls.end())
            {
                return it->second;
            }

            return nullptr;
        }
    };


    i_call_factory::u_ptr_t     m_factory;
    i_call_manager::u_ptr_t     m_manager;
    message_callback_t          m_message_callback;
    vgw_handle_t                m_manager_handle;

    call_container              m_calls;

public:

    static api_adapter& get_instance()
    {
        static api_adapter single_adapter;
        return single_adapter;
    }

    api_adapter()
        : m_factory(i_engine::get_instance().query_factory("opal"))
        , m_message_callback(nullptr)
        , m_manager_handle(0)
    {

    }

    ~api_adapter()
    {

    }

    vgw_handle_t create_manager(const vgw_call_manager_config_t& config
                                , message_callback_t message_callback)
    {
        if (m_manager == nullptr
                && message_callback != nullptr)
        {
            m_manager = m_factory->create_manager(detail::create_opal_manager_config(config));
            if (m_manager)
            {
                m_manager->set_listener(this);
                m_manager_handle++;
                m_message_callback = message_callback;
                return m_manager_handle;
            }
        }

        return vgw_failed;
    }

    inline vgw_result_t release_manager(vgw_handle_t handle)
    {
        if (m_manager != nullptr
                && m_manager_handle == handle)
        {
            m_manager.reset();
            m_message_callback = nullptr;
            return vgw_ok;
        }

        return vgw_failed;
    }

    inline vgw_result_t control_manager(vgw_handle_t handle, bool start)
    {
        if (m_manager != nullptr
                && m_manager_handle == handle)
        {
            if (start ? m_manager->start() : m_manager->stop())
            {
                return vgw_ok;
            }
        }

        return vgw_failed;
    }

    inline vgw_result_t make_call(vgw_handle_t handle, const char* remote_url)
    {
        if (m_manager != nullptr
                && m_manager_handle == handle)
        {
            if (m_manager->make_call(remote_url))
            {
                return vgw_ok;
            }
        }

        return vgw_failed;
    }

    inline vgw_result_t send_user_input(vgw_handle_t handle
                                        , const char* tones)
    {
        if (auto call = get_call(handle))
        {
            if (call->control(voip_control_tone_t(tones)))
            {
                return vgw_ok;
            }
        }

        return vgw_failed;
    }

    inline vgw_result_t hangup_call(vgw_handle_t handle)
    {
        if (auto call = get_call(handle))
        {
            if (call->control(voip_control_hangup_call_t()))
            {
                return vgw_ok;
            }
        }

        return vgw_failed;
    }

    inline i_call_manager* get_manager(vgw_handle_t handle)
    {
        if (m_manager_handle == handle)
        {
            return m_manager.get();
        }

        return nullptr;
    }

    inline i_call* get_call(vgw_handle_t handle)
    {
        return m_calls.get_call(handle);
    }

    vgw_result_t feedback_manager_message(vgw_manager_event_type_t event_type
                                          , i_call* call = nullptr)
    {
        if (m_message_callback)
        {
            vgw_manager_message_t manager_message = {};
            manager_message.event_type = event_type;
            std::string call_id;
            std::string call_url;

            switch(event_type)
            {
                case create_call:
                case release_call:
                {
                    call_id = call->id();
                    call_url = call->url();
                    manager_message.body.call_info.call_handle = call->index();
                    if (!call_id.empty())
                    {
                        manager_message.body.call_info.call_id = call_id.c_str();
                    }

                    if (!call_url.empty())
                    {
                        manager_message.body.call_info.remote_url = call_url.c_str();
                    }
                }
                break;
                default:;
            }

            return (*m_message_callback)(message_manager
                                        , &manager_message);
        }

        return vgw_failed;
    }

    vgw_result_t feedback_stream_message(i_media_stream &stream, bool open)
    {
        if (m_message_callback)
        {
            vgw_call_message_t call_message = {};
            call_message.event_type = open ? open_stream : close_stream;
            call_message.handle = stream.session().get_call().index();
            call_message.body.stream_info.media_type = static_cast<vgw_media_type_t>(stream.format().type());
            call_message.body.stream_info.stream_type = stream.type() == stream_type_t::inbound ? inbound : outbound;
            call_message.body.stream_info.stream_id = stream.session().id();

            return (*m_message_callback)(message_call
                                        , &call_message);

        }

        return vgw_failed;
    }

    vgw_result_t feedback_frame_message(i_media_stream &stream
                                        , const i_media_frame& frame
                                        , bool read)
    {
        if (m_message_callback)
        {
            vgw_call_message_t call_message = {};
            call_message.event_type = read ? read_frame : write_frame;
            call_message.handle = stream.session().get_call().index();
            detail::fill_c_frame(frame
                                 , stream
                                 , call_message.body.media_frame);

            auto result = (*m_message_callback)(message_call
                                                , &call_message);


            if (result > 0
                    && !read)
            {
                detail::merge_frame_info(const_cast<i_media_frame&>(frame)
                                         , call_message.body.media_frame);
            }

            return result;

        }

        return vgw_failed;
    }

    vgw_result_t feedback_user_input(i_call &call
                                     , const char* tones)
    {
        if (m_message_callback)
        {
            vgw_call_message_t call_message = {};
            call_message.event_type = user_input;
            call_message.handle = call.index();
            call_message.body.tones = tones;

            return (*m_message_callback)(message_call
                                         , &call_message);
        }

        return vgw_failed;
    }

    // i_listener interface
public:
    void on_add_stream(i_media_stream &stream) override
    {
        feedback_stream_message(stream, true);
    }

    void on_remove_stream(i_media_stream &stream) override
    {
        feedback_stream_message(stream, false);
    }

    std::size_t on_read_frame(i_media_stream &stream
                              , i_media_frame &frame) override
    {

        auto result = feedback_frame_message(stream
                                             , frame
                                             , true);
        return result > 0 ? result : 0;
    }

    std::size_t on_write_frame(i_media_stream &stream
                               , const i_media_frame &frame) override
    {
        auto result = feedback_frame_message(stream
                                             , frame
                                             , false);
        return result > 0 ? result : 0;
    }

    // i_listener interface
public:
    void on_add_session(i_media_session &session) override
    {
        session.set_listener(this);
    }

    void on_remove_session(i_media_session &session) override
    {
        session.set_listener(nullptr);
    }

    void on_control(i_call& call, const voip_control_t &control) override
    {
        switch(control.control_id)
        {
            case voip_control_id_t::tone:
                feedback_user_input(call
                                    , static_cast<const voip_control_tone_t&>(control).tone_string.c_str());
            break;
            default:;
        }
    }

    // i_listener interface
public:
    bool on_call(i_call &call
                 , i_call_manager::call_event_t event) override
    {
        switch(event)
        {
            case i_call_manager::call_event_t::new_call:
            {
                if (m_calls.add_call(&call))
                {
                    call.set_listener(this);
                    return feedback_manager_message(create_call, &call);
                }
            }
            break;
            case i_call_manager::call_event_t::close_call:
            {
                if (m_calls.remove_call(&call))
                {
                    return feedback_manager_message(release_call, &call);
                }
            }
            break;
            default:;
        }

        return false;
    }

    void on_started() override
    {
        feedback_manager_message(started);
    }

    void on_stopped() override
    {
        feedback_manager_message(stopped);
    }
};

}

vgw_handle_t create_manager(const vgw_call_manager_config_t* manager_config
                            , message_callback_t callback_manager)
{
    if (manager_config != nullptr
            && callback_manager != nullptr)
    {
        return voip::api_adapter::get_instance().create_manager(*manager_config
                                                                , callback_manager);
    }

    return vgw_failed;
}

vgw_result_t release_manager(vgw_handle_t manager_handle)
{
    return voip::api_adapter::get_instance().release_manager(manager_handle);
}

vgw_result_t start_manager(vgw_handle_t manager_handle)
{
    return voip::api_adapter::get_instance().control_manager(manager_handle
                                                             , true);
}

vgw_result_t stop_manager(vgw_handle_t manager_handle)
{
    return voip::api_adapter::get_instance().control_manager(manager_handle
                                                             , false);
}

vgw_result_t make_call(vgw_handle_t manager_handle
                       , const char* url)
{
    return voip::api_adapter::get_instance().make_call(manager_handle
                                                       , url);
}

vgw_result_t send_user_input(vgw_handle_t call_handle
                             , const char* tones)
{
    return voip::api_adapter::get_instance().send_user_input(call_handle
                                                             , tones);
}

vgw_result_t hangup_call(vgw_handle_t call_handle)
{
    return voip::api_adapter::get_instance().hangup_call(call_handle);
}
