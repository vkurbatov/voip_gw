#include "opal_media_stream.h"
#include <opal/mediafmt.h>
#include <opal/mediastrm.h>
#include <opal/mediasession.h>

#include "stream_metrics.h"
#include "opal_media_session.h"

namespace voip
{

opal_media_stream::opal_media_format::opal_media_format(OpalMediaStream &native_stream)
    : m_native_stream(native_stream)
{

}

media_type_t opal_media_stream::opal_media_format::type() const
{
    return opal_media_stream::get_media_type(m_native_stream);
}

uint32_t opal_media_stream::opal_media_format::sample_rate() const
{
    return m_native_stream.GetMediaFormat().GetClockRate();
}

uint32_t opal_media_stream::opal_media_format::channels() const
{
    return m_native_stream.GetMediaFormat().GetOptionInteger(OpalAudioFormat::ChannelsOption(), 1);
}

string opal_media_stream::opal_media_format::name() const
{
    return m_native_stream.GetMediaFormat().GetName();
}

media_type_t opal_media_stream::get_media_type(const OpalMediaStream &opal_stream)
{
    auto media_type = opal_stream.GetMediaFormat().GetMediaType();

    if (media_type == OpalMediaType::Audio())
    {
        return media_type_t::audio;
    }
    else if (media_type == OpalMediaType::Video())
    {
        return media_type_t::video;
    }

    return media_type_t::undefined;
}

opal_media_stream::u_ptr_t opal_media_stream::create(opal_media_session &session
                                                     , OpalMediaStream &native_stream)
{

    return std::make_unique<opal_media_stream>(session
                                               , native_stream);
}

opal_media_stream::opal_media_stream(opal_media_session &session
                                     , OpalMediaStream &native_stream)
    : m_session(session)
    , m_native_stream(native_stream)
    , m_format(m_native_stream)
    , m_opal_statistic(std::make_unique<OpalMediaStatistics>())
{
    m_session.on_add_stream(*this);
}

opal_media_stream::~opal_media_stream()
{
    m_session.on_remove_stream(*this);
}

OpalMediaStream &opal_media_stream::native_stream() const
{
    return m_native_stream;
}

stream_type_t opal_media_stream::type() const
{
    return m_native_stream.IsSource()
            ? stream_type_t::outbound
            : stream_type_t::inbound;
}

const i_media_format &opal_media_stream::format() const
{
    return m_format;
}

i_media_session& opal_media_stream::session() const
{
    return m_session;
}

bool opal_media_stream::get_metrics(stream_metrics_t &metrics) const
{
    if (m_opal_statistic)
    {
        OpalMediaStatistics& opal_statistic = *m_opal_statistic;
        opal_statistic.Update(m_native_stream);
        metrics.octets = opal_statistic.m_totalBytes;
        metrics.frames = opal_statistic.m_totalFrames;
        metrics.bitrate = opal_statistic.GetBitRate();
        metrics.framerate = opal_statistic.GetFrameRate();
        metrics.packets = opal_statistic.m_totalPackets;
        metrics.lost = opal_statistic.m_packetsLost;
        metrics.jitter_ms = opal_statistic.m_averageJitter;
        metrics.rtt_ms = opal_statistic.m_roundTripTime;
        metrics.format = std::string(opal_statistic.m_mediaFormat);

        switch(m_format.type())
        {
            case media_type_t::audio:
            {
                metrics.format.append(":")
                        .append(std::to_string(m_format.sample_rate()))
                        .append("/")
                        .append(std::to_string(m_format.channels()));
            }
            break;
            case media_type_t::video:
            {
                metrics.format.append(":")
                        .append(std::to_string(opal_statistic.m_frameWidth))
                        .append(":")
                        .append(std::to_string(opal_statistic.m_frameHeight));
            }
            break;
            default:;
        }

        return true;
    }
    return false;
}

}
