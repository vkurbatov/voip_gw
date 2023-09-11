#include "opal_media_session.h"
#include <opal/mediafmt.h>
#include <opal/mediastrm.h>

#include "opal_call.h"

namespace voip
{



opal_media_session::opal_media_session(opal_call& owner_call
                                       , std::size_t session_id
                                       , media_type_t media_type)
    : m_owner_call(owner_call)
    , m_session_id(session_id)
    , m_media_type(media_type)
{

}

opal_media_session::~opal_media_session()
{

}

bool opal_media_session::add_stream(OpalMediaStream &native_stream)
{
    if (native_stream.GetSessionID() == m_session_id)
    {
        auto& stream = native_stream.IsSource() ? m_outbound_stream : m_inbound_stream;

        stream.reset();

        stream = opal_media_stream::create(*this
                                           , native_stream);
    }

    return false;
}

bool opal_media_session::remove_stream(const OpalMediaStream &native_stream)
{
    if (m_inbound_stream != nullptr
            && &m_inbound_stream->native_stream() == &native_stream)
    {
        m_inbound_stream.reset();
        return true;
    }
    else if (m_outbound_stream != nullptr
             && &m_outbound_stream->native_stream() == &native_stream)
    {
        m_outbound_stream.reset();
        return true;
    }

    return false;
}

media_type_t opal_media_session::media_type() const
{
    return m_media_type;
}

std::size_t opal_media_session::id() const
{
    return m_session_id;
}

i_media_stream *opal_media_session::get_stream(stream_type_t stream_type) const
{
    switch(stream_type)
    {
        case stream_type_t::inbound:
            return m_inbound_stream.get();
        break;
        case stream_type_t::outbound:
            return m_outbound_stream.get();
        break;
        default:;
    }

    return nullptr;
}

i_call &opal_media_session::get_call() const
{
    return m_owner_call;
}

void opal_media_session::on_add_stream(opal_media_stream &stream)
{

}

void opal_media_session::on_remove_stream(opal_media_stream &stream)
{

}

}
