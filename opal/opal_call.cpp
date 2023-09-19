#include "opal_call.h"
#include <opal/connection.h>
#include <opal/call.h>

namespace voip
{

opal_call::opal_call(OpalConnection &native_connection
                     , call_direction_t direction
                     , i_listener* listener)
    : m_native_connection(native_connection)
    , m_direction(direction)
    , m_listener(listener)
{

}

bool opal_call::add_stream(OpalMediaStream &native_stream)
{
    if (auto session = query_opal_session(native_stream.GetSessionID()
                                         , opal_media_stream::get_media_type(native_stream))
            )
    {
        return session->add_stream(native_stream);
    }

    return false;
}

bool opal_call::remove_stream(const OpalMediaStream &native_stream)
{
    if (auto session = get_opal_session(native_stream.GetSessionID()))
    {
        if (session->remove_stream(native_stream))
        {
            if (session->is_empty())
            {
                remove_session(native_stream.GetSessionID());
            }
            return true;
        }
    }

    return false;
}

opal_media_session *opal_call::get_opal_session(std::size_t session_id)
{
    if (auto it = m_sessions.find(session_id); it != m_sessions.end())
    {
        return &it->second;
    }
    return nullptr;
}

OpalConnection &opal_call::native_connection() const
{
    return m_native_connection;
}

OpalCall &opal_call::native_call() const
{
    return m_native_connection.GetCall();
}

void opal_call::set_listener(i_listener *listener)
{
    m_listener = listener;
}

voip_protocol_t opal_call::protocol() const
{
    return voip_protocol_t::sip;
}

call_direction_t opal_call::direction() const
{
    return m_direction;
}

i_media_session *opal_call::get_session(std::size_t session_id)
{
    return get_opal_session(session_id);
}

size_t opal_call::session_count() const
{
    return m_sessions.size();
}

std::string opal_call::url() const
{
    return m_native_connection.GetCall().GetPartyB();
}

std::string opal_call::id() const
{
    return m_native_connection.GetCall().GetToken();
}

bool opal_call::is_established() const
{
    return m_native_connection.IsEstablished();
}

opal_media_session* opal_call::query_opal_session(std::size_t session_id
                                                  , media_type_t media_type)
{
    if (auto session = get_opal_session(session_id))
    {
        if (session->media_type() == media_type)
        {
            return session;
        }
    }
    else if (auto it = m_sessions.emplace(std::piecewise_construct
                                          , std::forward_as_tuple(session_id)
                                          , std::forward_as_tuple(*this
                                                                  , session_id
                                                                  , media_type)
                                          ); it.second)
    {
        return &it.first->second;
    }

    return nullptr;
}

bool opal_call::remove_session(std::size_t session_id)
{
    return m_sessions.erase(session_id) >= 0;
}

void opal_call::on_add_session(opal_media_session &session)
{
    if (m_listener)
    {
        m_listener->on_add_session(session);
    }
}

void opal_call::on_remove_session(opal_media_session &session)
{
    if (m_listener)
    {
        m_listener->on_remove_session(session);
    }
}


}
