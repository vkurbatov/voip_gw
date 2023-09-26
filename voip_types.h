#ifndef VOIP_TYPES_H
#define VOIP_TYPES_H

namespace voip
{

enum class voip_protocol_t
{
    undefined = 0,
    sip,
    h323,
    custom
};

enum class call_direction_t
{
    undefined,
    incoming,
    outgiong
};

enum class voip_control_id_t
{
    undefined,
    hangup_call,
    tone,
    custom
};

}

#endif // VOIP_TYPES_H
