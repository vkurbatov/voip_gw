#ifndef VOIP_I_CALL_FACTORY_H
#define VOIP_I_CALL_FACTORY_H

#include "i_call_manager.h"

namespace voip
{

struct call_manager_config_t;

class i_call_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_call_factory>;
    using s_ptr_t = std::shared_ptr<i_call_factory>;

    virtual ~i_call_factory() = default;
    virtual i_call_manager::u_ptr_t create_manager(const call_manager_config_t& config) = 0;
};

}

#endif // VOIP_I_CALL_FACTORY_H
