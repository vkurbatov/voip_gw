#ifndef VOIP_OPAL_CALL_H
#define VOIP_OPAL_CALL_H

#include "i_call.h"

class OpalConnection;
class OpalMediaStream;

namespace voip
{

class opal_call : public i_call
{
    OpalConnection&     m_native_connection;
    call_direction_t    m_direction;

public:
    opal_call(OpalConnection& native_connection
              , call_direction_t direction = call_direction_t::undefined);

    bool add_stream(OpalMediaStream& native_stream);
    bool remove_stream(const OpalMediaStream& native_stream);
    // i_call interface
public:
    voip_protocol_t protocol() const override;
    call_direction_t direction() const override;
    i_media_session *get_session(std::size_t session_id) override;
    std::size_t session_count() const override;
    std::string url() const override;
    std::string id() const override;
    bool is_established() const override;
};

}

#endif // VOIP_OPAL_CALL_H
