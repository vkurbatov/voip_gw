#ifndef VOIP_I_CALL_MANAGER_H
#define VOIP_I_CALL_MANAGER_H

#include "i_call.h"
#include <string_view>
#include "codec_info.h"

namespace voip
{

class i_call_manager
{
public:
    enum class call_event_t
    {
        new_call,
        close_call
    };

    class i_listener
    {
    public:
        virtual ~i_listener() = default;
        virtual bool on_call(i_call& call, call_event_t event) = 0;
        virtual void on_started() = 0;
        virtual void on_stopped() = 0;
    };

    using u_ptr_t = std::unique_ptr<i_call_manager>;
    using s_ptr_t = std::shared_ptr<i_call_manager>;

    virtual ~i_call_manager() = default;

    virtual void set_listener(i_listener* listener) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;
    virtual bool make_call(const std::string_view& url) = 0;
    virtual i_call* get_call(const std::string_view& call_id) = 0;
    virtual codec_info_t::array_t active_codecs() const = 0;

};

}

#endif // VOIP_I_CALL_MANAGER_H
