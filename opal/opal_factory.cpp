#include "opal_factory.h"
#include "opal_manager_config.h"
#include "opal_call.h"
#include "opal_types.h"
#include "opal_utils.h"

#include <ep/localep.h>
#include <sip/sipep.h>
#include <opal/transcoders.h>

#include <shared_mutex>
#include <thread>
#include <cstring>

namespace voip
{


class opal_manager final : public i_call_manager
{
    using shared_mutex_t = std::shared_mutex;
    using shared_lock_t = std::shared_lock<shared_mutex_t>;
    using lock_t = std::unique_lock<shared_mutex_t>;

    using sip_endpoint_ptr_t = std::unique_ptr<SIPEndPoint>;

    class opal_endpoint final : public OpalLocalEndPoint
    {
        opal_manager&   m_owner;

    public:
        opal_endpoint(opal_manager& owner)
            : OpalLocalEndPoint(owner.m_native_manager
                                , OPAL_LOCAL_PREFIX
                                , true)
            , m_owner(owner)
        {

        }

        // OpalEndPoint interface
    public:


        PBoolean OnOpenMediaStream(OpalConnection &connection
                                   , OpalMediaStream &stream) override
        {
            return m_owner.on_open_media_stream(connection
                                                , stream);
        }

        void OnClosedMediaStream(const OpalMediaStream &stream) override
        {
            m_owner.on_close_media_stream(stream.GetConnection()
                                         , stream);
        }

        // OpalLocalEndPoint interface
    public:

        bool OnIncomingCall(OpalLocalConnection &connection) override
        {
            if (m_owner.on_incoming_call(connection))
            {
                return OpalLocalEndPoint::OnIncomingCall(connection);
            }
            return false;
        }

        bool OnOutgoingCall(const OpalLocalConnection &connection) override
        {
            if (m_owner.on_outgoing_call(const_cast<OpalLocalConnection&>(connection)))
            {
                return OpalLocalEndPoint::OnOutgoingCall(connection);
            }

            return false;
        }

        bool OnReadMediaData(const OpalLocalConnection &connection
                             , const OpalMediaStream &mediaStream
                             , void *data
                             , PINDEX size
                             , PINDEX &length) override
        {
            return m_owner.on_read_media_data(connection
                                               , mediaStream
                                               , data
                                               , size
                                               , length);
        }

        bool OnWriteMediaData(const OpalLocalConnection &connection
                              , const OpalMediaStream &mediaStream
                              , const void *data
                              , PINDEX length
                              , PINDEX &written) override
        {

            return m_owner.on_write_media_data(connection
                                               , mediaStream
                                               , data
                                               , length
                                               , written);
        }


        void OnReleased(OpalConnection &connection) override
        {
            m_owner.on_released_call(connection);
        }

        // OpalLocalEndPoint interface
    public:
        bool OnUserInput(const OpalLocalConnection &connection, const PString &indication) override
        {
            m_owner.on_user_input(connection
                                  , indication);

            return true;
        }
    };

    using call_map_t = std::unordered_map<std::string, opal_call>;

    mutable shared_mutex_t      m_safe_mutex;

    opal_manager_config_t       m_config;
    i_listener*                 m_listener;
    OpalManager                 m_native_manager;
    opal_endpoint               m_endpoint;
    sip_endpoint_ptr_t          m_sip_endpoint;

    call_map_t                  m_calls;

public:

    using u_ptr_t = std::unique_ptr<opal_manager>;

    static sip_endpoint_ptr_t create_sip_endpoint(OpalManager& native_manager)
    {
        if (auto sipep = std::make_unique<SIPEndPoint>(native_manager))
        {
            if (sipep->StartListeners(PStringArray()))
            {
                return sipep;
            }
        }

        return nullptr;
    }

    static u_ptr_t create(const call_manager_config_t &config)
    {
        if (config.is_valid()
                && config.type == opal_type)
        {
            return std::make_unique<opal_manager>(static_cast<const opal_manager_config_t&>(config));
        }

        return nullptr;
    }

    opal_manager(const opal_manager_config_t& config)
        : m_config(config)
        , m_listener(nullptr)
        , m_endpoint(*this)
    {
        set_manager_config(m_config);
        m_native_manager.AddRouteEntry("sip.*:.* = local:");
        m_native_manager.AddRouteEntry("local:.* = si:<da>");

    }

    ~opal_manager() override
    {
        stop();
    }

    // i_call_manager interface
public:
    void set_listener(i_listener *listener) override
    {
        if (m_sip_endpoint == nullptr)
        {
            m_listener = listener;
        }
    }

    bool start() override
    {
        if (m_sip_endpoint == nullptr)
        {
            if (m_listener)
            {
                m_sip_endpoint = create_sip_endpoint(m_native_manager);

                if (m_sip_endpoint != nullptr)
                {
                    m_listener->on_started();
                    return true;
                }
            }
        }

        return false;
    }

    bool stop() override
    {
        if (m_sip_endpoint != nullptr)
        {
            m_listener->on_stopped();
            m_sip_endpoint.reset();
            m_calls.clear();
            return true;
        }

        return false;
    }

    bool is_started() const override
    {
        return m_sip_endpoint != nullptr;
    }

    bool make_call(const std::string_view &url) override
    {
        PString token;
        return m_native_manager.SetUpCall("local:*", url.data(), token);
    }


    i_call *get_call(const string_view &call_id) override
    {
        return get_opal_call(std::string(call_id));
    }

    codec_info_t::array_t active_codecs() const override
    {
        codec_info_t::array_t codecs;
        const PStringArray& mask  = m_native_manager.GetMediaFormatMask();
        std::set<std::string> mask_set;
        for (auto i = 0; i < mask.GetSize(); i++)
        {
            mask_set.insert(mask[i]);
        }

        for (const auto& f : OpalTranscoder::GetPossibleFormats(m_endpoint.GetMediaFormats()))
        {
            if (f.IsTransportable())
            {
                if (auto enc_name = f.GetEncodingName())
                {
                    if (mask_set.find(f.GetName()) == mask_set.end())
                    {
                        codecs.emplace_back(get_media_type(f.GetMediaType())
                                            , enc_name
                                            , std::string(f.GetName())
                                            , std::string(f.GetDescription())
                                            , f.GetClockRate());
                    }
                }
            }
        }
        return codecs;
    }

private:

    void set_manager_config(const opal_manager_config_t& config)
    {
        if (!config.user_name.empty())
        {
            m_native_manager.SetDefaultUserName(config.user_name);
        }

        if (!config.display_name.empty())
        {
            m_native_manager.SetDefaultDisplayName(config.display_name);
        }

        if (config.min_rtp_port > 0
                && config.min_rtp_port < config.max_rtp_port)
        {
            m_native_manager.SetRtpIpPorts(config.min_rtp_port, config.max_rtp_port);
        }

        if (config.max_rtp_packet_size > 0)
        {
            m_native_manager.SetMaxRtpPayloadSize(config.max_rtp_packet_size);
        }

        if (config.max_bitrate > 0)
        {
            m_endpoint.SetInitialBandwidth(OpalBandwidth::Direction::Rx
                                                , OpalBandwidth(config.max_bitrate));

            m_endpoint.SetInitialBandwidth(OpalBandwidth::Direction::Tx
                                                , OpalBandwidth(config.max_bitrate));
        }


        auto is_mask_audio = !config.audio_codecs.empty();
        auto is_mask_video = !config.video_codecs.empty();

        if (is_mask_audio
                || is_mask_video)
        {
            std::set<std::string> mask_set;
            for (const auto& ac : config.audio_codecs)
            {
                mask_set.insert(ac);
            }

            for (const auto& vc : config.video_codecs)
            {
                mask_set.insert(vc);
            }

            PStringArray mask;

            for (const auto& f : OpalTranscoder::GetPossibleFormats(m_endpoint.GetMediaFormats()))
            {
                if (f.IsTransportable())
                {
                    if (auto enc_name = f.GetEncodingName())
                    {
                        switch(get_media_type(f.GetMediaType()))
                        {
                            case media_type_t::audio:
                            {
                                if (is_mask_audio)
                                {
                                    if (mask_set.find(enc_name) == mask_set.end())
                                    {
                                        mask += f.GetName();
                                    }
                                }
                            }
                            break;
                            case media_type_t::video:
                            {
                                if (is_mask_video)
                                {
                                    if (mask_set.find(enc_name) == mask_set.end())
                                    {
                                        mask += f.GetName();
                                    }
                                }
                            }
                            break;
                            default:;
                        }
                    }
                }
            }

            m_native_manager.SetMediaFormatMask(mask);

        }


        // std::set<std::string> codec_map;

    }

    inline opal_call* get_opal_call(const std::string& token)
    {
        std::shared_lock lock(m_safe_mutex);

        if (auto it = m_calls.find(token); it != m_calls.end())
        {
            return &it->second;
        }

        return nullptr;
    }

    inline opal_call* get_opal_call(const OpalConnection &connection)
    {
        return get_opal_call(connection.GetToken());
    }

    inline opal_call* create_call(OpalConnection &connection
                                  , call_direction_t direction)
    {
        std::unique_lock lock(m_safe_mutex);
        if (auto it = m_calls.emplace(std::piecewise_construct
                                      , std::forward_as_tuple(std::string(connection.GetToken()))
                                      , std::forward_as_tuple(connection)
                               ); it.second)
        {

            on_call(it.first->second
                    , call_event_t::new_call);
            return &it.first->second;
        }

        return nullptr;
    }

    inline bool remove_call(const std::string& token)
    {
        std::unique_lock lock(m_safe_mutex);
        if (auto it = m_calls.find(token)
                ; it != m_calls.end())
        {
            on_call(it->second
                    , call_event_t::close_call);
            m_calls.erase(it);
            return true;
        }
        return false;
    }

    inline void on_user_input(const OpalConnection& connection
                              , const std::string& indication)
    {
        if (auto call = get_opal_call(connection))
        {
            call->on_user_input(indication);
        }
    }

    inline bool on_incoming_call(OpalConnection &connection)
    {
        return create_call(connection
                           , call_direction_t::incoming) != nullptr;
    }

    inline bool on_outgoing_call(OpalConnection &connection)
    {
        return create_call(connection
                           , call_direction_t::outgiong) != nullptr;
    }

    inline void on_call(opal_call& call
                        , call_event_t event)
    {
        if (m_listener)
        {
            m_listener->on_call(call
                                , event);
        }
    }

    inline void on_released_call(OpalConnection &connection)
    {
        remove_call(connection.GetToken());
    }

    bool on_open_media_stream(const OpalConnection &connection
                              , OpalMediaStream &stream)
    {
        if (auto call = get_opal_call(connection))
        {
            return call->add_stream(stream);
        }

        return false;
    }

    void on_close_media_stream(const OpalConnection &connection
                               , const OpalMediaStream &stream)
    {
        if (auto call = get_opal_call(connection))
        {
            call->remove_stream(stream);
        }
    }

    bool on_read_media_data(const OpalConnection &connection
                            , const OpalMediaStream &mediaStream
                            , void *data
                            , PINDEX size
                            , PINDEX &length)
    {
        if (auto call = get_opal_call(connection))
        {
            if (auto session = call->get_opal_session(mediaStream.GetSessionID()))
            {
                length = session->read_data(data
                                            , size);
                return length > 0;
            }
        }

        return false;
    }

    bool on_write_media_data(const OpalLocalConnection &connection
                            , const OpalMediaStream &mediaStream
                            , const void *data
                            , PINDEX length
                            , PINDEX &written)
    {
        if (auto call = get_opal_call(connection))
        {
            if (auto session = call->get_opal_session(mediaStream.GetSessionID()))
            {
                written = session->write_data(data
                                              , length);
                return written > 0;
            }
        }

        return false;
    }
};

class opal_process : public PProcess
{
public:
    using u_ptr_t = std::unique_ptr<opal_process>;
    static u_ptr_t create()
    {
        return std::make_unique<opal_process>();
    }

    opal_manager::u_ptr_t create_manager(const call_manager_config_t &config)
    {
        if (IsInitialised())
        {
            return opal_manager::create(config);
        }
        return nullptr;
    }
    // PThread interface
public:
    void Main() override
    {

    }

};

opal_factory::u_ptr_t opal_factory::create()
{
    return std::make_unique<opal_factory>();
}

opal_factory &opal_factory::get_instance()
{
    static opal_factory single_factory;
    return single_factory;
}

opal_factory::opal_factory()
    : m_opal_process(opal_process::create())
{
    m_opal_process->InternalMain();
}

opal_factory::~opal_factory()
{

}

i_call_manager::u_ptr_t opal_factory::create_manager(const call_manager_config_t &config)
{
    return m_opal_process->create_manager(config);
}

codec_info_t::array_t opal_factory::supported_codecs()
{
    return get_supported_opal_codecs(media_type_t::undefined);
}

}
