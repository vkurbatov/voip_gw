#ifndef VOIP_OPAL_FACTORY_H
#define VOIP_OPAL_FACTORY_H

#include "i_call_factory.h"

class PProcess;

namespace voip
{

class opal_process;

class opal_factory : public i_call_factory
{
    std::unique_ptr<opal_process>   m_opal_process;
public:

    using u_ptr_t = std::unique_ptr<opal_factory>;
    using s_ptr_t = std::shared_ptr<opal_factory>;


    static u_ptr_t create();
    static opal_factory& get_instance();
    opal_factory();
    ~opal_factory() override;

    // i_call_factory interface
public:
    i_call_manager::u_ptr_t create_manager(const call_manager_config_t &config) override;
    codec_info_t::array_t supported_codecs() override;
};

}

#endif // VOIP_OPAL_FACTORY_H
