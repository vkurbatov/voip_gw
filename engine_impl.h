#ifndef VOIP_ENGINE_IMPL_H
#define VOIP_ENGINE_IMPL_H

#include "i_engine.h"

namespace voip
{

class engine_impl : public i_engine
{
    // i_engine interface
public:

    // static engine_impl& get_instance();

    i_call_factory::u_ptr_t query_factory(const std::string_view &engine_type) override;
};

}

#endif // VOIP_ENGINE_IMPL_H
