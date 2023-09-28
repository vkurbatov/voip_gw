#include "api_test.h"

#include "opal/opal_manager_config.h"
#include "i_engine.h"
#include "i_media_session.h"
#include "i_media_stream.h"
#include "i_video_frame.h"
#include "i_audio_frame.h"

#include "utils/frame_queue.h"

#include "voip_control.h"
#include "stream_metrics.h"

#include <thread>
#include <iostream>
#include <cstring>

using namespace voip;

class frame_queue_adapter : public frame_queue
{
public:
    frame_queue_adapter()
        : frame_queue(5)
    {

    }

    std::size_t push_frame(const i_media_frame& frame)
    {
        std::size_t result = 0;

        if (auto clone_frame = frame.clone())
        {
            result = clone_frame->size();
            frame_queue::push_frame(std::move(clone_frame));
        }

        return result;
    }

    std::size_t pop_frame(i_media_frame& dst_frame
                          , std::uint32_t timeout_ms = 0)
    {
        if (auto src_frame = frame_queue::pop_frame(timeout_ms))
        {
            if (src_frame->type() == dst_frame.type())
            {
                switch(dst_frame.type())
                {
                    case media_type_t::audio:
                    {
                        auto& audio_frame_dst = static_cast<i_audio_frame&>(dst_frame);
                        auto& audio_frame_src = static_cast<i_audio_frame&>(*src_frame);

                        if (audio_frame_dst.format() == audio_frame_src.format()
                                && audio_frame_dst.size() >= audio_frame_src.size())
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
                        auto& video_frame_dst = static_cast<i_video_frame&>(dst_frame);
                        auto& video_frame_src = static_cast<i_video_frame&>(*src_frame);

                        if (video_frame_dst.format() == video_frame_src.format()
                                && video_frame_dst.size() >= video_frame_dst.size())
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
};


class test_listener: public i_call_manager::i_listener
        , public i_call::i_listener
        , public i_media_session::i_listener
{
    frame_queue_adapter     m_audio_queue;
    frame_queue_adapter     m_video_queue;

    i_call*                 m_call = nullptr;


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

    frame_queue_adapter* get_queue(std::size_t session_id)
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
            return queue->pop_frame(frame
                                    , 500);
        }

        return 0;
    }

    std::size_t on_write_frame(i_media_stream &stream
                               , const i_media_frame &frame) override
    {
        if (auto queue = get_queue(stream.session().id()))
        {
            return queue->push_frame(frame);
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
    void on_control(i_call& call, const voip_control_t &control) override
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

    auto factory = i_engine::get_instance().query_factory("opal");

    config.max_bitrate = 1024000; // not work!!!

    config.audio_codecs.push_back("G7221");
    config.video_codecs.push_back("H264");

    config.min_rtp_port = 10000;
    config.max_rtp_port = 10100;

    config.max_rtp_packet_size = 900;

    auto count = 1000;

    if (auto manager = factory->create_manager(config))
    {
        std::cout << "Supported codecs: " << std::endl;
        for (const auto& c : manager->active_codecs())
        {
            std::cout << (c.type == media_type_t::audio ? "audio" : (c.type == media_type_t::undefined ? "undefined" : "video"))
                      << ", name: " << c.name
                      << ", full name: " << c.full_name
                      << ", desc: " << c.description
                      << ", sample_rate: " << c.sample_rate
                      << std::endl;
        }

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
