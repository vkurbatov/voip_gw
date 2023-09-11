#include "call_manager_config.h"

namespace voip
{

call_manager_config_t::call_manager_config_t(const std::string_view &type)
    : type(type)
{

}

bool call_manager_config_t::operator ==(const call_manager_config_t &other) const
{
    return type == other.type;
}

bool call_manager_config_t::operator !=(const call_manager_config_t &other) const
{
    return !operator == (other);
}

bool call_manager_config_t::is_valid() const
{
    return false;
}



}
