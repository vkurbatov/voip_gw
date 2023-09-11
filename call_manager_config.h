#ifndef VOIP_CALL_MANAGER_CONFIG_H
#define VOIP_CALL_MANAGER_CONFIG_H

#include <string>

namespace voip
{

struct call_manager_config_t
{
    std::string     type;

    call_manager_config_t(const std::string_view& type = {});

    virtual bool operator == (const call_manager_config_t& other) const;
    virtual bool operator != (const call_manager_config_t& other) const;

    virtual bool is_valid() const;
};

}

#endif // VOIP_CALL_MANAGER_CONFIG_H
