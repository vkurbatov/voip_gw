#include "voip_control.h"

namespace voip
{

voip_control_t::voip_control_t(voip_control_id_t control_id
                               , const std::string_view &control_name)
    : control_id(control_id)
    , control_name(control_name)
{

}

bool voip_control_t::is_valid() const
{
    return control_id != voip_control_id_t::undefined;
}

voip_control_hangup_call_t::voip_control_hangup_call_t()
    : voip_control_t(voip_control_id_t::hangup_call
                     , "hangup_call")
{

}

bool voip_control_hangup_call_t::is_valid() const
{
    return control_id == voip_control_id_t::hangup_call;
}

voip_control_tone_t::voip_control_tone_t(const std::string_view &tone_string)
    : voip_control_t(voip_control_id_t::tone
                     , "tone")
    , tone_string(tone_string)
{

}

bool voip_control_tone_t::is_valid() const
{
    return control_id == voip_control_id_t::tone
            && !tone_string.empty();
}



}
