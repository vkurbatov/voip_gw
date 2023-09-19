#ifndef VOIP_OPAL_MEDIA_STREAM_H
#define VOIP_OPAL_MEDIA_STREAM_H

#include "i_media_stream.h"
#include "i_media_format.h"

class OpalMediaStream;

namespace voip
{

class opal_media_session;

class opal_media_stream : public i_media_stream
{
    class opal_media_format : public i_media_format
    {
        OpalMediaStream&    m_native_stream;
    public:
        opal_media_format(OpalMediaStream& native_stream);
        // i_media_format interface
    public:
        media_type_t type() const override;
        uint32_t sample_rate() const override;
        uint32_t channels() const override;
        std::string name() const override;
    };

    opal_media_session&     m_session;
    OpalMediaStream&        m_native_stream;
    opal_media_format       m_format;

public:

    using u_ptr_t = std::unique_ptr<opal_media_stream>;
    using s_ptr_t = std::shared_ptr<opal_media_stream>;

    static media_type_t get_media_type(const OpalMediaStream& opal_stream);

    static u_ptr_t create(opal_media_session& session
                          , OpalMediaStream& native_stream);

    opal_media_stream(opal_media_session& session
                      , OpalMediaStream& native_stream);

    ~opal_media_stream();

    OpalMediaStream& native_stream() const;   

    // i_media_stream interface
public:
    stream_type_t type() const override;
    const i_media_format &format() const override;
    i_media_session &session() const override;

};

}

#endif // VOIP_OPAL_MEDIA_STREAM_H
