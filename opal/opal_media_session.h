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

    i_listener*                 m_listener;

    opal_media_stream::u_ptr_t  m_inbound_stream;
    opal_media_stream::u_ptr_t  m_outbound_stream;

    friend class opal_media_stream;

public:

    using u_ptr_t = std::unique_ptr<opal_media_session>;

    static u_ptr_t create(opal_call& owner_call
                          , std::size_t session_id = 0
                          , media_type_t media_type = media_type_t::undefined
                          , i_listener* listener = nullptr);

    opal_media_session(opal_call& owner_call
                       , std::size_t session_id = 0
                       , media_type_t media_type = media_type_t::undefined
                       , i_listener* listener = nullptr);

    ~opal_media_session();

    bool add_stream(OpalMediaStream& native_stream);
    bool remove_stream(const OpalMediaStream& native_stream);

    opal_call& get_opal_call() const;
    opal_media_stream* get_opal_stream(stream_type_t stream_type) const;
    opal_media_stream* get_opal_stream(const OpalMediaStream& opal_stream) const;

    std::size_t write_data(const void* data
                           , std::size_t size);

    std::size_t read_data(void* data
                          , std::size_t size);

    // no streams
    bool is_empty() const;

    // i_media_session interface
public:
    void set_listener(i_listener* listener) override;
    media_type_t media_type() const override;
    std::size_t id() const override;
    i_media_stream *get_stream(stream_type_t stream_type) const override;
    i_call& get_call() const override;

private:
    void on_add_stream(opal_media_stream& stream);
    void on_remove_stream(opal_media_stream& stream);
    std::size_t on_read_frame(opal_media_stream& stream
                              , i_media_frame& frame);
    std::size_t on_write_frame(opal_media_stream& stream
                                , const i_media_frame& frame);

};

}

#endif // VOIP_OPAL_MEDIA_SESSION_H
