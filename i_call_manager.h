#ifndef VOIP_I_CALL_MANAGER_H
#define VOIP_I_CALL_MANAGER_H

#include "i_call.h"
#include <string_view>

namespace voip
{

class i_call_manager
{
public:
    class i_listener
    {
        virtual ~i_listener() = default;
        virtual bool on_call(i_call& call) = 0;
    };

    using u_ptr_t = std::unique_ptr<i_call_manager>;
    using s_ptr_t = std::shared_ptr<i_call_manager>;
    virtual ~i_call_manager() = default;

    virtual void set_listener(i_listener* listener) = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;
    virtual bool make_call(const std::string_view& url) = 0;

};

}

#endif // VOIP_I_CALL_MANAGER_H
