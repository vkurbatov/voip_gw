#ifndef VOIP_I_MEDIA_STREAM_H
#define VOIP_I_MEDIA_STREAM_H

#include "media_types.h"
#include <memory>

namespace voip
{

class i_media_format;
class i_media_session;
struct stream_metrics_t;

class i_media_stream
{
public:
    using u_ptr_t = std::unique_ptr<i_media_stream>;
    using s_ptr_t = std::shared_ptr<i_media_stream>;

    virtual ~i_media_stream() = default;

    virtual stream_type_t type() const = 0;
    virtual const i_media_format& format() const = 0;
    virtual i_media_session& session() const = 0;
    virtual bool get_metrics(stream_metrics_t& metrics) const = 0;
};

}

#endif // VOIP_I_MEDIA_STREAM_H
