#include "common_utils.h"

namespace voip::utils
{

std::vector<std::string> split_string(const std::string_view &source_string
                                      , const std::string_view &delimeter)
{
    std::vector<std::string> split;

    if (!delimeter.empty())
    {
        std::size_t pos = 0;

        auto s = source_string;

        while((pos = s.find_first_of(delimeter, 0)) != std::string::npos)
        {
            split.emplace_back(s.substr(0, pos));
            s = s.substr(pos + delimeter.length());
        }
        split.emplace_back(s);
    }

    if (split.empty() && !source_string.empty())
    {
        split.emplace_back(source_string);
    }
    return split;
}


}
