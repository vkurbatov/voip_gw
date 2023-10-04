#ifndef VOIP_I_ENGINE_H
#define VOIP_I_ENGINE_H

#include "i_call_factory.h"

namespace voip
{

class i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_engine>;
    using s_ptr_t = std::shared_ptr<i_engine>;
    using r_ptr_t = i_engine*;

    static i_engine& get_instance();

    virtual ~i_engine() = default;
    virtual i_call_factory::u_ptr_t query_factory(const std::string_view& type) = 0;
};

}

#endif // VOIP_I_ENGINE_H
