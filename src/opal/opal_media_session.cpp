#include "opal_media_session.h"
#include <opal/mediafmt.h>
#include <opal/mediastrm.h>

#include "opal_video_frame.h"
#include "audio_frame_impl.h"

#include "mutable_data_buffer.h"
#include "const_data_buffer.h"

#include "opal_call.h"

namespace voip
{

namespace detail
{

audio_frame_info_t create_audio_info(const i_media_format& format)
{
    return { format.name()
           , format.sample_rate()
           , format.channels()
           };
}

}

opal_media_session::u_ptr_t opal_media_session::create(opal_call &owner_call
                                                       , std::size_t session_id
                                                       , media_type_t media_type
                                                        , i_listener* listener)
{
    return std::make_unique<opal_media_session>(owner_call
                                                , session_id
                                                , media_type
                                                , listener);
}

opal_media_session::opal_media_session(opal_call& owner_call
                                       , std::size_t session_id
                                       , media_type_t media_type
                                       , i_listener* listener)
    : m_owner_call(owner_call)
    , m_session_id(session_id)
    , m_media_type(media_type)
    , m_listener(listener)
{
    m_owner_call.on_add_session(*this);
}

opal_media_session::~opal_media_session()
{
    m_owner_call.on_remove_session(*this);
}

bool opal_media_session::add_stream(OpalMediaStream &native_stream)
{
    if (native_stream.GetSessionID() == m_session_id)
    {
        auto& stream = native_stream.IsSource() ? m_outbound_stream : m_inbound_stream;

        stream.reset();

        stream = opal_media_stream::create(*this
                                           , native_stream);

        return stream != nullptr;
    }

    return false;
}

bool opal_media_session::remove_stream(const OpalMediaStream &native_stream)
{
    auto& stream = native_stream.IsSource() ? m_outbound_stream : m_inbound_stream;

    if (stream != nullptr
            && &stream->native_stream() == &native_stream)
    {
        stream.reset();
        return true;
    }

    return false;
}

opal_call &opal_media_session::get_opal_call() const
{
    return m_owner_call;
}

opal_media_stream *opal_media_session::get_opal_stream(stream_type_t stream_type) const
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

opal_media_stream *opal_media_session::get_opal_stream(const OpalMediaStream &opal_stream) const
{
    return get_opal_stream(opal_stream.IsSource()
                            ? stream_type_t::outbound
                            : stream_type_t::inbound);
}

size_t opal_media_session::write_data(const void *data
                                      , std::size_t size)
{
    const_data_buffer buffer(data
                             , size);
    if (auto stream = get_opal_stream(stream_type_t::inbound))
    {
        std::size_t result = 0;
        switch(m_media_type)
        {
            case media_type_t::audio:
            {
                audio_frame_impl_ref audio_frame(buffer
                                                 , detail::create_audio_info(stream->format())
                                                 );
                result = on_write_frame(*stream
                                        , audio_frame);
            }
            break;
            case media_type_t::video:
            {
                opal_video_frame_ref video_frame(buffer
                                                 , stream->format().name());

                if (auto write_size = on_write_frame(*stream
                                                     , video_frame))
                {
                    result = write_size + opal_video_frame_ref::opal_video_header_size();
                }
            }
            break;
            default:;
        }

        return result;
    }
    return 0;
}

size_t opal_media_session::read_data(void *data
                                     , std::size_t size)
{
    mutable_data_buffer buffer(data
                               , size);
    if (auto stream = get_opal_stream(stream_type_t::outbound))
    {
        std::size_t result = 0;
        switch(m_media_type)
        {
            case media_type_t::audio:
            {
                audio_frame_impl_ref audio_frame(buffer
                                                 , detail::create_audio_info(stream->format())
                                                 );
                result = on_read_frame(*stream
                                       , audio_frame);
            }
            break;
            case media_type_t::video:
            {
                opal_video_frame_ref video_frame(buffer
                                                 , stream->format().name());

                if (auto read_size = on_read_frame(*stream
                                                   , video_frame))
                {
                    result = read_size + opal_video_frame_ref::opal_video_header_size();
                }
            }
            break;
            default:;
        }


        return result;
    }
    return 0;
}

bool opal_media_session::is_empty() const
{
    return m_inbound_stream == nullptr
            && m_outbound_stream == nullptr;
}

void opal_media_session::set_listener(i_listener *listener)
{
    m_listener = listener;
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
    return get_opal_stream(stream_type);
}

i_call &opal_media_session::get_call() const
{
    return get_opal_call();
}

void opal_media_session::on_add_stream(opal_media_stream &stream)
{
    if (m_listener)
    {
        m_listener->on_add_stream(stream);
    }
}

void opal_media_session::on_remove_stream(opal_media_stream &stream)
{
    if (m_listener)
    {
        m_listener->on_remove_stream(stream);
    }
}

std::size_t opal_media_session::on_read_frame(opal_media_stream &stream
                                              , i_media_frame &frame)
{
    if (m_listener)
    {
        return m_listener->on_read_frame(stream
                                        , frame);
    }

    return 0;
}

std::size_t opal_media_session::on_write_frame(opal_media_stream &stream
                                               , const i_media_frame &frame)
{
    if (m_listener)
    {
        return m_listener->on_write_frame(stream
                                          , frame);
    }

    return 0;
}

}
