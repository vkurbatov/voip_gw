#include "engine_impl.h"
#include "opal/opal_factory.h"
#include "opal/opal_types.h"

namespace voip
{

i_engine& i_engine::get_instance()
{
    static engine_impl single_engine;
    return single_engine;
}

i_call_factory::u_ptr_t engine_impl::query_factory(const std::string_view &engine_type)
{
    if (engine_type == opal_engine_type)
    {
        return opal_factory::create();
    }

    return nullptr;
}


}
