#include "voip_api.h"

#include "i_engine.h"
#include "i_media_session.h"
#include "i_media_stream.h"
#include "i_media_format.h"
#include "i_media_frame.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"

#include "video_frame_impl.h"
#include "audio_frame_impl.h"
#include "smart_buffer.h"

#include "voip_control.h"

#include "utils/common_utils.h"
#include "utils/frame_queue.h"

#include "opal/opal_manager_config.h"

#include <unordered_map>
#include <mutex>
#include <cstring>

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
                         , vgw_media_frame_t& c_frame)
{
    switch(frame.type())
    {
        case media_type_t::audio:
        {
            auto& audio_frame = static_cast<const i_audio_frame&>(frame);
            c_frame.media_type = vgw_media_type_audio;
            c_frame.info.audio_info.format = audio_frame.format().c_str();
            c_frame.info.audio_info.sample_rate = audio_frame.sample_rate();
            c_frame.info.audio_info.channels = audio_frame.channels();
        }
        break;
        case media_type_t::video:
        {
            auto& video_frame = static_cast<const i_video_frame&>(frame);
            c_frame.media_type = vgw_media_type_video;
            c_frame.info.video_info.format = video_frame.format().c_str();
            c_frame.info.video_info.width = video_frame.width();
            c_frame.info.video_info.height = video_frame.height();
        }
        break;
        default:;
    }

    if (c_frame.buffer_ptr == nullptr
            || c_frame.buffer_size == 0)
    {
        c_frame.buffer_ptr = const_cast<void*>(frame.data());
        c_frame.buffer_size = frame.size();
    }
    else
    {
        if (c_frame.buffer_size > frame.size())
        {
            c_frame.buffer_size = frame.size();
        }

        std::memcpy(c_frame.buffer_ptr
                    , frame.data()
                    , c_frame.buffer_size);
    }
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

i_media_frame::u_ptr_t create_frame(const vgw_media_frame_t& frame)
{
    if (frame.buffer_size > 0
            && frame.buffer_ptr != nullptr)
    {
        smart_buffer buffer(frame.buffer_ptr
                            , frame.buffer_size);

        switch (frame.media_type)
        {
            case vgw_media_type_audio:
            {
                audio_frame_info_t audio_info;
                if (frame.info.audio_info.format)
                {
                    audio_info.format = frame.info.audio_info.format;
                }
                audio_info.sample_rate = frame.info.audio_info.sample_rate;
                audio_info.channels = frame.info.audio_info.channels;

                if (audio_info.is_valid())
                {
                    buffer.make_store();
                    return audio_frame_impl::create(std::move(buffer)
                                                    , audio_info);
                }
            }
            break;
            case vgw_media_type_video:
            {
                video_frame_info_t video_info;
                if (frame.info.video_info.format)
                {
                    video_info.format = frame.info.video_info.format;
                }
                video_info.width = frame.info.video_info.width;
                video_info.height = frame.info.video_info.height;

                if (video_info.is_valid())
                {
                    buffer.make_store();
                    return video_frame_impl::create(std::move(buffer)
                                                    , video_info);
                }
            }
            break;
            default:;
        }
    }
    return nullptr;
}

}

class queue_manager
{
    class queue : public frame_queue
    {
        i_media_frame::u_ptr_t  m_last_frame;
    public:
        queue(uint32_t queue_size)
            : frame_queue(queue_size)
        {

        }

        vgw_result_t push_frame(const vgw_media_frame_t& media_frame)
        {
            vgw_result_t result = vgw_failed;
            if (auto frame = detail::create_frame(media_frame))
            {
                result = frame->size();
                frame_queue::push_frame(std::move(frame));
            }

            return result;
        }

        vgw_result_t pop_frame(vgw_media_frame_t& media_frame
                               , std::uint32_t timeout_ms)
        {
            vgw_result_t result = vgw_failed;

            if (auto frame = frame_queue::pop_frame(timeout_ms))
            {
                m_last_frame = std::move(frame);
                detail::fill_c_frame(*m_last_frame
                                     , media_frame);
                media_frame.buffer_size = m_last_frame->size();
                media_frame.buffer_ptr = m_last_frame->map();
                result = m_last_frame->size();
            }

            return result;
        }

        vgw_result_t clear()
        {
            vgw_result_t result = stats().pending;
            reset();
            return result;
        }
    };
    using mutex_t = std::mutex;
    using queue_map_t = std::unordered_map<vgw_handle_t, queue>;

    mutable mutex_t     m_safe_mutex;
    queue_map_t         m_queues;
    vgw_handle_t        m_handles;

public:

    static queue_manager& get_instance()
    {
        static queue_manager single_adapter;
        return single_adapter;
    }

    queue_manager()
        : m_handles(0)
    {

    }


    inline vgw_handle_t create_queue(std::uint32_t queue_size)
    {
        std::lock_guard lock(m_safe_mutex);
        auto handle = m_handles++;

        m_queues.emplace(handle
                         , queue_size);

        return handle;
    }

    inline vgw_handle_t release_queue(vgw_handle_t handle)
    {
        std::lock_guard lock(m_safe_mutex);
        return m_queues.erase(handle) > 0
                ? vgw_ok
                : vgw_failed;
    }

    inline vgw_handle_t clear_queue(vgw_handle_t handle)
    {
        if (auto queue = get_queue(handle))
        {
            return queue->clear();
        }

        return vgw_failed;
    }

    inline vgw_result_t push_frame(vgw_handle_t handle
                                   , const vgw_media_frame_t& media_frame)
    {
        if (auto queue = get_queue(handle))
        {
            return queue->push_frame(media_frame);
        }

        return vgw_failed;
    }

    inline vgw_result_t pop_frame(vgw_handle_t handle
                                   , vgw_media_frame_t& media_frame
                                   , uint32_t timeout_ms = 0)
    {
        if (auto queue = get_queue(handle))
        {
            return queue->pop_frame(media_frame
                                    , timeout_ms);
        }

        return vgw_failed;
    }

private:
    queue* get_queue(vgw_handle_t handle)
    {
        std::lock_guard lock(m_safe_mutex);

        if (auto it = m_queues.find(handle); it != m_queues.end())
        {
            return &it->second;
        }

        return nullptr;
    }
};

class manager_adapter: public i_call_manager::i_listener
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

        inline bool add_call(i_call* call)
        {
            std::lock_guard lock(m_mutex);
            return m_calls.emplace(call->index()
                                    , call).second;
        }

        inline bool remove_call(i_call* call)
        {
            std::lock_guard lock(m_mutex);
            return m_calls.erase(call->index()) > 0;
        }

        inline i_call* get_call(vgw_handle_t handle)
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
    void*                       m_user_data;
    vgw_handle_t                m_manager_handle;

    call_container              m_calls;

public:

    static manager_adapter& get_instance()
    {
        static manager_adapter single_adapter;
        return single_adapter;
    }

    manager_adapter()
        : m_factory(i_engine::get_instance().query_factory("opal"))
        , m_message_callback(nullptr)
        , m_manager_handle(0)
    {

    }

    ~manager_adapter()
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
                case vgw_manager_event_create_call:
                case vgw_manager_event_release_call:
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

                    manager_message.body.call_info.call_type = static_cast<vgw_call_type_t>(call->direction());
                }
                break;
                default:;
            }

            return (*m_message_callback)(vgw_message_type_manager
                                        , &manager_message);
        }

        return vgw_failed;
    }

    vgw_result_t feedback_stream_message(i_media_stream &stream, bool open)
    {
        if (m_message_callback)
        {
            vgw_call_message_t call_message = {};
            call_message.event_type = open ? vgw_call_event_open_stream : vgw_call_event_close_stream;
            call_message.handle = stream.session().get_call().index();
            call_message.body.stream_info.media_type = static_cast<vgw_media_type_t>(stream.format().type());
            call_message.body.stream_info.stream_type = stream.type() == stream_type_t::inbound ? vgw_stream_type_inbound : vgw_stream_type_outbound;
            call_message.body.stream_info.stream_id = stream.session().id();

            return (*m_message_callback)(vgw_message_type_call
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
            call_message.event_type = read ? vgw_call_event_read_frame : vgw_call_event_write_frame;
            call_message.handle = stream.session().get_call().index();
            detail::fill_c_frame(frame
                                 , call_message.body.media_frame);

            call_message.body.media_frame.stream_id = stream.session().id();

            auto result = (*m_message_callback)(vgw_message_type_call
                                                , &call_message);


            if (result > 0
                    && read)
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
            call_message.event_type = vgw_call_event_user_input;
            call_message.handle = call.index();
            call_message.body.tones = tones;

            return (*m_message_callback)(vgw_message_type_call
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
                    return feedback_manager_message(vgw_manager_event_create_call, &call);
                }
            }
            break;
            case i_call_manager::call_event_t::close_call:
            {
                if (m_calls.remove_call(&call))
                {
                    return feedback_manager_message(vgw_manager_event_release_call, &call);
                }
            }
            break;
            default:;
        }

        return false;
    }

    void on_started() override
    {
        feedback_manager_message(vgw_manager_event_started);
    }

    void on_stopped() override
    {
        feedback_manager_message(vgw_manager_event_stopped);
    }
};

}

vgw_handle_t vgw_create_manager(const vgw_call_manager_config_t* manager_config
                                , message_callback_t callback_manager)
{
    if (manager_config != nullptr
            && callback_manager != nullptr)
    {
        return voip::manager_adapter::get_instance().create_manager(*manager_config
                                                                    , callback_manager);
    }

    return vgw_failed;
}

vgw_result_t vgw_release_manager(vgw_handle_t manager_handle)
{
    return voip::manager_adapter::get_instance().release_manager(manager_handle);
}

vgw_result_t vgw_start_manager(vgw_handle_t manager_handle)
{
    return voip::manager_adapter::get_instance().control_manager(manager_handle
                                                             , true);
}

vgw_result_t vgw_stop_manager(vgw_handle_t manager_handle)
{
    return voip::manager_adapter::get_instance().control_manager(manager_handle
                                                             , false);
}

vgw_result_t vgw_make_call(vgw_handle_t manager_handle
                       , const char* url)
{
    return voip::manager_adapter::get_instance().make_call(manager_handle
                                                       , url);
}

vgw_result_t vgw_send_user_input(vgw_handle_t call_handle
                             , const char* tones)
{
    return voip::manager_adapter::get_instance().send_user_input(call_handle
                                                             , tones);
}

vgw_result_t vgw_hangup_call(vgw_handle_t call_handle)
{
    return voip::manager_adapter::get_instance().hangup_call(call_handle);
}

vgw_handle_t vgw_create_frame_queue(uint32_t queue_size)
{
    return voip::queue_manager::get_instance().create_queue(queue_size);
}

vgw_handle_t vgw_release_frame_queue(vgw_handle_t handle)
{
    return voip::queue_manager::get_instance().release_queue(handle);
}

vgw_handle_t vgw_clear_frame_queue(vgw_handle_t handle)
{
    return voip::queue_manager::get_instance().clear_queue(handle);
}

vgw_result_t vgw_push_frame(vgw_handle_t handle
                            , const struct vgw_media_frame_t* media_frame)
{
    return voip::queue_manager::get_instance().push_frame(handle
                                                          , *media_frame);
}

vgw_result_t vgw_pop_frame(vgw_handle_t handle
                           , struct vgw_media_frame_t* media_frame
                           , uint32_t timeout_ms)
{
    return voip::queue_manager::get_instance().pop_frame(handle
                                                         , *media_frame
                                                         , timeout_ms);
}
