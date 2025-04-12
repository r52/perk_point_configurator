#include "config.h"

#include <REX/REX/LOG.h>

#include <string>

namespace PPC
{
    void Config::LoadConfig()
    {
        mINI::INIFile      file("./Data/F4SE/Plugins/ppc.ini");
        mINI::INIStructure ini;

        if (file.read(ini))
        {
            for (auto const& it : ini)
            {
                auto const& collection = it.second;
                for (auto const& it2 : collection)
                {
                    auto const& key      = it2.first;
                    auto const& value    = it2.second;

                    auto        keyParts = Util::SplitString<std::vector<std::string>>(key, "-");

                    if (keyParts.size() < 1 || keyParts.size() > 2)
                    {
                        REX::WARN("incorrect range value '{}', found {} parts", key, keyParts.size());
                        continue;
                    }

                    auto low  = std::stoul(keyParts[0]);
                    auto high = (keyParts.size() == 2) ? std::stoul(keyParts[1]) : low;

                    if (!(low <= high))
                    {
                        REX::WARN("level range must be from low to high in '{}', low = {}, high = {}", key, low, high);
                        continue;
                    }

                    auto rate = std::stof(value);

                    if (!(rate >= 0.0f && rate < 128.0f))
                    {
                        REX::WARN("invalid perk count rate '{}', must be between 0 and 128", value);
                        continue;
                    }

                    REX::INFO("registering range({}, {}) = {}", low, high, rate);
                    rates.insert_or_assign(Range((std::uint16_t) low, (std::uint16_t) high), rate);
                }
            }
        }
        else
        {
            REX::WARN("failed to read ppc.ini");
        }
    }
}  // namespace PPC