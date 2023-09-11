#include "opal_manager_config.h"
#include "opal_types.h"

namespace voip
{

opal_manager_config_t::opal_manager_config_t()
    : call_manager_config_t(opal_type)
{

}

bool opal_manager_config_t::operator ==(const call_manager_config_t &other) const
{
    return other.type == opal_type;
}

bool opal_manager_config_t::is_valid() const
{
    return type == opal_type;
}

}
