#ifndef VOIP_I_MEDIA_SESSION_H
#define VOIP_I_MEDIA_SESSION_H

#include "media_types.h"
#include <memory>

namespace voip
{

class i_media_stream;

class i_media_session
{
public:
    using u_ptr_t = std::unique_ptr<i_media_session>;
    using s_ptr_t = std::shared_ptr<i_media_session>;

    virtual ~i_media_session() = default;
    virtual media_type_t media_type() const = 0;
    virtual std::size_t id() const = 0;
    virtual i_media_stream* get_stream(stream_type_t stream_type) const = 0;
};

}

#endif // VOIP_I_MEDIA_SESSION_H
