#include "opal_call.h"
#include <opal/connection.h>
#include <opal/call.h>

namespace voip
{

opal_call::opal_call(OpalConnection &native_connection
                     , call_direction_t direction)
    : m_native_connection(native_connection)
    , m_direction(direction)
{

}

bool opal_call::add_stream(OpalMediaStream &native_stream)
{
    return false;
}

bool opal_call::remove_stream(const OpalMediaStream &native_stream)
{
    return false;
}

voip_protocol_t opal_call::protocol() const
{
    return voip_protocol_t::sip;
    // m_native_connection.GetCall().Get
}

call_direction_t opal_call::direction() const
{
    return m_direction;
}

i_media_session *opal_call::get_session(std::size_t session_id)
{
    return nullptr;
}

size_t opal_call::session_count() const
{
    return 0;
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

}
