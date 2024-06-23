#pragma once

#include <utility>

#include <F4SE/Logger.h>
#include <RE/Bethesda/BSTEvent.h>
#include <RE/Bethesda/Events.h>
#include <RE/Bethesda/PlayerCharacter.h>

namespace PPC
{
    inline constexpr REL::Version version(Version::MAJOR, Version::MINOR, Version::PATCH);

    // hopefully non-clashing unique ID
    inline constexpr std::uint32_t id = 0x23E6CC73;

    void                           OnSaveCallback(const F4SE::SerializationInterface* a_intfc);
    void                           OnLoadCallback(const F4SE::SerializationInterface* a_intfc);
    void                           OnRevertCallback(const F4SE::SerializationInterface* a_intfc);

    void                           F4SEMessageHandler(F4SE::MessagingInterface::Message* msg);

    class PerkState
    {
    public:
        [[nodiscard]] static PerkState* GetSingleton()
        {
            static PerkState singleton;
            return &singleton;
        }

        void SetState(RE::PlayerCharacter* player) { SetState((std::uint16_t) player->GetLevel(), player->perkCount); }

        void SetState(const std::uint16_t& level, const std::int8_t& perkCount)
        {
            logger::debug("Saving PlayerCharacter state: Level {}, perk count {}", level, perkCount);
            _level     = level;
            _perkCount = perkCount;
        }

        auto GetState() const { return std::make_pair(_level, _perkCount); }

        void SetPerkProgress(const float& progress)
        {
            logger::debug("Saving perk progress: {}", progress);
            _perkProgress = progress;
        }

        const float& GetPerkProgress() const { return _perkProgress; }

        PerkState(PerkState const&)       = delete;
        void operator= (PerkState const&) = delete;

    private:
        PerkState() = default;

        // New game defaults
        std::uint16_t _level        = 1;
        std::int8_t   _perkCount    = 0;
        float         _perkProgress = 0.0f;
    };

    class PerkPointEventSink : public RE::BSTEventSink<RE::PerkPointIncreaseEvent>
    {
    public:
        virtual ~PerkPointEventSink() override = default;
        virtual RE::BSEventNotifyControl ProcessEvent(const RE::PerkPointIncreaseEvent& event, RE::BSTEventSource<RE::PerkPointIncreaseEvent>* source) override;
    };
}  // namespace PPC
