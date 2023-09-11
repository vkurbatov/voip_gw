#ifndef VOIP_OPAL_FACTORY_H
#define VOIP_OPAL_FACTORY_H

#include "i_call_factory.h"

namespace voip
{

class opal_factory : public i_call_factory
{
public:

    using u_ptr_t = std::unique_ptr<opal_factory>;
    using s_ptr_t = std::shared_ptr<opal_factory>;

    static u_ptr_t create();

    opal_factory();

    // i_call_factory interface
public:
    i_call_manager::u_ptr_t create_manager(const call_manager_config_t &config) override;
};

}

#endif // VOIP_OPAL_FACTORY_H
