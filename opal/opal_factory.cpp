#include "opal_factory.h"
#include "opal_manager_config.h"
#include "opal_types.h"

#include <ep/localep.h>
#include <sip/sipep.h>

namespace voip
{


class opal_manager : public i_call_manager
{
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

    };

    opal_manager_config_t       m_config;
    i_listener*                 m_listener;
    OpalManager                 m_native_manager;
    opal_endpoint               m_endpoint;
    sip_endpoint_ptr_t          m_sip_endpoint;

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
