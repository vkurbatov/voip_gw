#ifndef VOIP_MEDIA_FRAME_H
#define VOIP_MEDIA_FRAME_H

#include "i_data_buffer.h"
#include "media_types.h"
#include <memory>

namespace voip
{

class i_media_frame : public i_data_buffer
{
public:
    using u_ptr_t = std::unique_ptr<i_media_frame>;
    using s_ptr_t = std::shared_ptr<i_media_frame>;

    virtual ~i_media_frame() = default;
    virtual media_type_t type() const = 0;
    virtual bool is_valid() const = 0;
};

}

#endif // VOIP_MEDIA_FRAME_H
