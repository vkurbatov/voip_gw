#ifndef VOIP_I_MEDIA_FORMAT_H
#define VOIP_I_MEDIA_FORMAT_H

#include "media_types.h"
#include <memory>

namespace voip
{

class i_media_format
{
public:
    using u_ptr_t = std::unique_ptr<i_media_format>;
    using s_ptr_t = std::shared_ptr<i_media_format>;

    virtual ~i_media_format() = default;
    virtual media_type_t type() const = 0;
    virtual std::uint32_t sample_rate() const = 0;
    virtual std::uint32_t channels() const = 0;
    virtual std::string name() const = 0;
};

}

#endif // VOIP_I_MEDIA_FORMAT_H
