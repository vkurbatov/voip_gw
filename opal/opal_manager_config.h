#ifndef VOIP_OPAL_MANAGER_CONFIG_H
#define VOIP_OPAL_MANAGER_CONFIG_H

#include "call_manager_config.h"

namespace voip
{

struct opal_manager_config_t : public call_manager_config_t
{
    opal_manager_config_t();

    // call_manager_config_t interface
public:
    bool operator ==(const call_manager_config_t &other) const override;
    bool is_valid() const override;
};

}

#endif // VOIP_OPAL_MANAGER_CONFIG_H
