#include "opal_factory.h"
#include "opal_manager_config.h"
#include "opal_call.h"
#include "opal_types.h"

#include <ep/localep.h>
#include <sip/sipep.h>

#include <shared_mutex>

namespace voip
{


class opal_manager : public i_call_manager
{
    using shared_mutex_t = std::shared_mutex;
    using shared_lock_t = std::shared_lock<shared_mutex_t>;
    using lock_t = std::unique_lock<shared_mutex_t>;

    using sip_endpoint_ptr_t = std::unique_ptr<SIPEndPoint>;

    class opal_endpoint : public OpalLocalEndPoint
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
        m_native_manager.AddRouteEntry("sip.*:.* = local:");
        m_native_manager.AddRouteEntry("local:.* = si:<da>");
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
        if (m_sip_endpoint = nullptr)
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
        return m_native_manager.SetUpCall("pc:*", url.data(), token);
    }


private:

    inline opal_call* get_call(const std::string& token)
    {
        std::shared_lock lock(m_safe_mutex);

        if (auto it = m_calls.find(token); it != m_calls.end())
        {
            return &it->second;
        }

        return nullptr;
    }

    inline opal_call* get_call(const OpalConnection &connection)
    {
        return get_call(connection.GetToken());
    }

    inline opal_call* create_call(OpalConnection &connection
                                  , call_direction_t direction)
    {
        std::unique_lock lock(m_safe_mutex);
        if (auto it = m_calls.emplace(std::piecewise_construct
                                      , std::forward_as_tuple(std::string(connection.GetToken()))
                                      , std::forward_as_tuple(connection
                                                              , call_direction_t::incoming)
                               ); it.second)
        {
            return &it.first->second;
        }

        return nullptr;
    }

    inline bool remove_call(const std::string& token)
    {
        std::unique_lock lock(m_safe_mutex);
        return m_calls.erase(token) > 0;
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

    inline void on_released_call(OpalConnection &connection)
    {
        remove_call(connection.GetToken());
    }

    bool on_open_media_stream(const OpalConnection &connection
                              , OpalMediaStream &stream)
    {
        if (auto call = get_call(connection))
        {
            return call->add_stream(stream);
        }

        return false;
    }

    void on_close_media_stream(const OpalConnection &connection
                               , const OpalMediaStream &stream)
    {
        if (auto call = get_call(connection))
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
        if (auto call = get_call(connection))
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
        if (auto call = get_call(connection))
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

opal_factory::u_ptr_t opal_factory::create()
{
    return std::make_unique<opal_factory>();
}

opal_factory::opal_factory()
{

}

i_call_manager::u_ptr_t opal_factory::create_manager(const call_manager_config_t &config)
{
    return opal_manager::create(config);
}

}
