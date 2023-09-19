#ifndef VOIP_I_MEDIA_SESSION_H
#define VOIP_I_MEDIA_SESSION_H

#include "media_types.h"
#include <memory>

namespace voip
{

class i_call;
class i_media_stream;
class i_media_frame;

class i_media_session
{
public:

    class i_listener
    {
    public:
        virtual ~i_listener() = 0;
        virtual void on_add_stream(i_media_stream& stream) = 0;
        virtual void on_remove_stream(const i_media_stream& stream) = 0;

        virtual std::size_t on_read_frame(i_media_stream& stream
                                          , i_media_frame& frame) = 0;
        virtual std::size_t on_write_frame(i_media_stream& stream
                                           , const i_media_frame& frame) = 0;
    };

    using u_ptr_t = std::unique_ptr<i_media_session>;
    using s_ptr_t = std::shared_ptr<i_media_session>;

    virtual ~i_media_session() = default;
    virtual void set_listener(i_listener* listener) = 0;
    virtual media_type_t media_type() const = 0;
    virtual std::size_t id() const = 0;
    virtual i_media_stream* get_stream(stream_type_t stream_type) const = 0;
    virtual i_call& get_call() const = 0;
};

}

#endif // VOIP_I_MEDIA_SESSION_H
