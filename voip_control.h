#ifndef VOIP_CONTROL_H
#define VOIP_CONTROL_H

#include "voip_types.h"
#include <string>

namespace voip
{

struct voip_control_t
{
    const voip_control_id_t control_id;
    const std::string       control_name;
protected:
    voip_control_t(voip_control_id_t control_id = voip_control_id_t::undefined
                    , const std::string_view& control_name = {});
public:
    virtual bool is_valid() const;
};

struct voip_control_hangup_call : public voip_control_t
{
    voip_control_hangup_call();

public:
    bool is_valid() const override;
};

struct voip_control_tone_t : public voip_control_t
{
    std::string         tone_string;
    voip_control_tone_t(const std::string_view& tone_string = {});

public:
    bool is_valid() const override;
};


}

#endif // VOIP_CONTROL_H
