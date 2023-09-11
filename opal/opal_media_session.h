#ifndef VOIP_OPAL_MEDIA_SESSION_H
#define VOIP_OPAL_MEDIA_SESSION_H

#include "i_media_session.h"
#include "opal_media_stream.h"

class OpalMediaStream;

namespace voip
{

class opal_call;

class opal_media_session : public i_media_session
{
    opal_call&                  m_owner_call;
    std::size_t                 m_session_id;
    media_type_t                m_media_type;
    opal_media_stream::u_ptr_t  m_inbound_stream;
    opal_media_stream::u_ptr_t  m_outbound_stream;

    friend class opal_media_stream;

public:
    opal_media_session(opal_call& owner_call
                       , std::size_t session_id = 0
                       , media_type_t media_type = media_type_t::undefined);

    ~opal_media_session();

    bool add_stream(OpalMediaStream& native_stream);
    bool remove_stream(const OpalMediaStream& native_stream);

    // i_media_session interface
public:
    media_type_t media_type() const override;
    std::size_t id() const override;
    i_media_stream *get_stream(stream_type_t stream_type) const override;
    i_call &get_call() const override;
private:
    void on_add_stream(opal_media_stream& stream);
    void on_remove_stream(opal_media_stream& stream);

};

}

#endif // VOIP_OPAL_MEDIA_SESSION_H
