#ifndef VOIP_I_CALL_H
#define VOIP_I_CALL_H

#include "voip_types.h"
#include <memory>
#include <string>

namespace voip
{

class i_media_session;
class i_media_stream;
struct voip_control_t;

class i_call
{
public:
    class i_listener
    {
    public:
        virtual ~i_listener() = default;
        virtual void on_add_session(i_media_session& session) = 0;
        virtual void on_remove_session(i_media_session& session) = 0;
        virtual void on_control(i_call& call, const voip_control_t& control) = 0;
    };

    using u_ptr_t = std::unique_ptr<i_call>;
    using s_ptr_t = std::shared_ptr<i_call>;

    virtual ~i_call() = default;
    virtual void set_listener(i_listener* listener) = 0;
    virtual voip_protocol_t protocol() const = 0;
    virtual call_direction_t direction() const = 0;
    virtual i_media_session* get_session(std::size_t session_id) = 0;
    virtual std::size_t session_count() const = 0;
    virtual std::string url() const = 0;
    virtual std::string id() const = 0;
    virtual bool is_established() const = 0;
    virtual std::size_t index() const = 0;
    virtual bool control(const voip_control_t& control) = 0;


};

}

#endif // VOIP_I_CALL_H
