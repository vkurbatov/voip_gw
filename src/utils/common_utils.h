#ifndef VOIP_COMMON_UTILS_H
#define VOIP_COMMON_UTILS_H

#include <string>
#include <vector>

namespace voip::utils
{

std::vector<std::string> split_string(const std::string_view& source_string
                                      , const std::string_view& delimeter);

}

#endif // VOIP_COMMON_UTILS_H
