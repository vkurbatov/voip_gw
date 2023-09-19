#ifndef VOIP_OPAL_CALL_H
#define VOIP_OPAL_CALL_H

#include "i_call.h"
#include "opal_media_session.h"
#include <unordered_map>

class OpalConnection;
class OpalCall;
class OpalMediaStream;

namespace voip
{

class opal_call : public i_call
{
    using session_map_t = std::unordered_map<std::uint32_t, opal_media_session>;

    OpalConnection&     m_native_connection;
    call_direction_t    m_direction;

    i_listener*         m_listener;

    session_map_t       m_sessions;

    friend class opal_media_session;

public:
    opal_call(OpalConnection& native_connection
              , call_direction_t direction = call_direction_t::undefined
              , i_listener* listener = nullptr);

    bool add_stream(OpalMediaStream& native_stream);
    bool remove_stream(const OpalMediaStream& native_stream);

    opal_media_session* get_opal_session(std::size_t session_id);

    OpalConnection& native_connection() const;
    OpalCall& native_call() const;

    // i_call interface
public:
    void set_listener(i_listener *listener) override;
    voip_protocol_t protocol() const override;
    call_direction_t direction() const override;
    i_media_session *get_session(std::size_t session_id) override;
    std::size_t session_count() const override;
    std::string url() const override;
    std::string id() const override;
    bool is_established() const override;

private:

    opal_media_session* query_opal_session(std::size_t session_id
                                            , media_type_t media_type);

    bool remove_session(std::size_t session_id);

    void on_add_session(opal_media_session& session);
    void on_remove_session(opal_media_session& session);

};

}

#endif // VOIP_OPAL_CALL_H
