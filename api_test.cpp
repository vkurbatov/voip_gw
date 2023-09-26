#include "test.h"

#include "opal/opal_factory.h"
#include "opal/opal_manager_config.h"
#include "i_media_session.h"
#include "i_media_stream.h"
#include "i_video_frame.h"
#include "i_audio_frame.h"

#include "voip_control.h"
#include "stream_metrics.h"

#include <thread>
#include <shared_mutex>
#include <iostream>
#include <queue>
#include <condition_variable>
#include <cstring>

namespace voip
{

class frame_queue
{

    constexpr static std::size_t max_queue_size = 5;

    using frame_queue_t = std::queue<i_media_frame::u_ptr_t>;

    mutable std::mutex          m_safe_mutex;
    frame_queue_t               m_queue;
    std::condition_variable     m_signal;

public:
    frame_queue()
    {

    }

    ~frame_queue()
    {

    }

    std::size_t push_frame(const i_media_frame& frame)
    {
        std::lock_guard lock(m_safe_mutex);
        if (auto clone_frame = frame.clone())
        {
            auto size = clone_frame->size();
            m_queue.emplace(std::move(std::move(clone_frame)));

            while(m_queue.size() > max_queue_size)
            {
                m_queue.pop();
            }

            m_signal.notify_all();

            return size;
        }
        return 0;
    }

    std::size_t pop_frame(i_media_frame& frame
                          , std::uint32_t timeout_ms = 0)
    {
        std::unique_lock lock(m_safe_mutex);

        if (timeout_ms > 0
                && m_queue.empty())
        {
            m_signal.wait_for(lock
                              , std::chrono::milliseconds(timeout_ms));
        }

        if (!m_queue.empty())
        {
            auto qframe = std::move(m_queue.front());
            m_queue.pop();

            if (qframe != nullptr
                    && qframe->type() == frame.type())
            {
                switch(frame.type())
                {
                    case media_type_t::audio:
                    {
                        auto& audio_frame_dst = static_cast<i_audio_frame&>(frame);
                        auto& audio_frame_src = static_cast<i_audio_frame&>(*qframe);

                        if (audio_frame_dst.size() >= audio_frame_src.size()
                                && audio_frame_dst.format() == audio_frame_src.format())
                        {
                            audio_frame_dst.set_sample_rate(audio_frame_src.sample_rate());
                            audio_frame_dst.set_channels(audio_frame_src.channels());

                            if (auto data_dst = audio_frame_dst.map())
                            {
                                std::memcpy(data_dst
                                            , audio_frame_src.data()
                                            , audio_frame_src.size());

                                return audio_frame_src.size();
                            }
                        }
                    }
                    break;
                    case media_type_t::video:
                    {
                        auto& video_frame_dst = static_cast<i_video_frame&>(frame);
                        auto& video_frame_src = static_cast<i_video_frame&>(*qframe);

                        if (video_frame_dst.size() >= video_frame_src.size()
                                && video_frame_dst.format() >= video_frame_src.format())
                        {
                            video_frame_dst.set_left(video_frame_src.left());
                            video_frame_dst.set_top(video_frame_src.top());
                            video_frame_dst.set_width(video_frame_src.width());
                            video_frame_dst.set_height(video_frame_src.height());

                            if (auto data_dst = video_frame_dst.map())
                            {
                                std::memcpy(data_dst
                                            , video_frame_src.data()
                                            , video_frame_src.size());

                                return video_frame_src.size();
                            }
                        }
                    }
                    break;
                    default:;
                }
            }
        }
        return 0;
    }

    std::size_t pending_frames() const
    {
        std::lock_guard lock(m_safe_mutex);
        return m_queue.size();
    }
};

class test_listener: public i_call_manager::i_listener
        , public i_call::i_listener
        , public i_media_session::i_listener
{
    frame_queue     m_audio_queue;
    frame_queue     m_video_queue;

    i_call*         m_call = nullptr;


public:

    test_listener()
    {

    }

    ~test_listener() override
    {

    }

    i_call* get_call() const
    {
        return m_call;
    }

    frame_queue* get_queue(std::size_t session_id)
    {
        switch(session_id)
        {
            case 1:
                return &m_audio_queue;
            break;
            case 2:
                return &m_video_queue;
            break;
            default:;
        }

        return 0;
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
                m_call = &call;
                std::cout << "Call: new call: " << call.id() << ", url: " << call.url() << std::endl;
                call.set_listener(this);
            }
            break;
            case i_call_manager::call_event_t::close_call:
            {
                m_call = nullptr;
                std::cout << "Call: close call: " << call.id() << ", url: " << call.url() << std::endl;
                call.set_listener(nullptr);
            }
            break;
            default:;
        }

        return true;
    }

    void on_started() override
    {
        std::cout << "Call manager: started" << std::endl;
    }

    void on_stopped() override
    {
        std::cout << "Call manager: stopped" << std::endl;
    }

    // i_listener interface
public:
    void on_add_stream(i_media_stream &stream) override
    {
        std::cout << "Media Session: add stream: " << stream.session().id() << std::endl;
    }

    void on_remove_stream(i_media_stream &stream) override
    {
        std::cout << "Media Session: remove stream: " << stream.session().id() << std::endl;
    }

    std::size_t on_read_frame(i_media_stream &stream
                              , i_media_frame &frame) override
    {
        if (auto queue = get_queue(stream.session().id()))
        {
            if (auto result = queue->pop_frame(frame
                                               , 100))
            {
                return result;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        return 0;
    }

    std::size_t on_write_frame(i_media_stream &stream
                               , const i_media_frame &frame) override
    {
        if (auto queue = get_queue(stream.session().id()))
        {
            if (auto result = queue->push_frame(frame))
            {
                return result;
            }
        }
        return 0;
    }

    // i_listener interface
public:
    void on_add_session(i_media_session &session) override
    {
        std::cout << "Call: add session: " << session.id() << std::endl;
        session.set_listener(this);
    }

    void on_remove_session(i_media_session &session) override
    {
        std::cout << "Call: remove session: " << session.id() << std::endl;
        session.set_listener(nullptr);
    }

    // i_listener interface
public:
    void on_control(const voip_control_t &control) override
    {
        switch(control.control_id)
        {
            case voip_control_id_t::tone:
                std::cout << "OnTone: " << static_cast<const voip_control_tone_t&>(control).tone_string << std::endl;
            break;
            default:;
        }
    }
};

void api_test()
{
    test_listener listener;

    opal_manager_config_t config;

    opal_factory factory;

    auto count = 1000;

    if (auto manager = factory.create_manager(config))
    {
        manager->set_listener(&listener);
        if (manager->start())
        {
            manager->make_call("sip:username@192.168.0.104:5060");
            std::this_thread::sleep_for(std::chrono::seconds(1));
            while(count-- > 0)
            {
                if (auto call = listener.get_call())
                {
                    call->control(voip_control_tone_t("1234"));
                    for (std::size_t i = 0; i < call->session_count(); i++)
                    {
                        if (auto session = call->get_session(i + 1))
                        {
                            for (const auto& t : { stream_type_t::inbound, stream_type_t::outbound})
                            {
                                if (auto stream = session->get_stream(t))
                                {
                                    stream_metrics_t metrics;
                                    if (stream->get_metrics(metrics))
                                    {
                                        std::cout << "Call {" << call->id() << "}: "
                                                  << "sid: " << session->id()
                                                  << ", stream: " << (t == stream_type_t::inbound ? "inbound" : "outbound")
                                                  << std::endl << "metrics: " << std::endl
                                                  << "octets: " << metrics.octets << std::endl
                                                  << "frames: " << metrics.frames << std::endl
                                                  << "bitrate: " << metrics.bitrate << std::endl
                                                  << "framerate: " << metrics.framerate << std::endl
                                                  << "packets: " << metrics.packets << std::endl
                                                  << "lost: " << metrics.lost << std::endl
                                                  << "jitter_ms: " << metrics.jitter_ms << std::endl
                                                  << "rtt_ms: " << metrics.rtt_ms << std::endl
                                                  << "format: " << metrics.format << std::endl
                                                  << std::endl;
                                    }
                                }
                            }
                        }
                    }
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            manager->stop();
        }
    }
}

}
