#pragma once

#include <regex>
#include <set>
#include <string>
#include <vector>

namespace Util
{
    template<typename T>
    T SplitString(const std::string& src, const std::string& delimiter)
        requires std::is_same_v<std::vector<std::string>, T> || std::is_same_v<std::set<std::string>, T>
    {
        const std::regex del{delimiter};
        T                list(std::sregex_token_iterator(src.begin(), src.end(), del, -1), {});

        std::erase_if(list, [](const std::string& s) {
            return s.empty();
        });

        return list;
    }
};  // namespace Util