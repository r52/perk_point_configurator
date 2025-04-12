#pragma once

#include <REX/REX/Singleton.h>

#include <assert.h>
#include <map>
#include <mini/ini.h>
#include <ranges>

namespace PPC
{
    struct Range
    {
        Range(std::uint16_t low, std::uint16_t high) : _begin((assert(low <= high), low)), _end(high) {}

        inline bool          operator== (const Range&) const = default;

        inline bool          operator< (const Range& rhs) const { return this->cend() < rhs.cbegin(); }

        bool                 inRange(std::uint16_t value) const { return _begin <= value && value <= _end; }

        std::uint16_t*       begin() { return &_begin; }

        std::uint16_t*       end() { return &_end; }

        const std::uint16_t& cbegin() const { return _begin; }

        const std::uint16_t& cend() const { return _end; }

        std::uint16_t        _begin;
        std::uint16_t        _end;
    };

    static_assert(std::ranges::range<Range>);

    class Config : public REX::Singleton<Config>
    {
    public:
        void                   LoadConfig();

        std::map<Range, float> rates;
    };
}  // namespace PPC