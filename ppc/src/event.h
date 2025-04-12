#pragma once

#include <utility>

#include <RE/Bethesda/BSTEvent.h>
#include <RE/Bethesda/Events.h>
#include <RE/Bethesda/PlayerCharacter.h>
#include <REX/REX/LOG.h>

namespace PPC
{
    inline constexpr REL::Version version(Version::MAJOR, Version::MINOR, Version::PATCH);

    // hopefully non-clashing unique ID
    inline constexpr std::uint32_t id = 0x23E6CC73;

    void                           OnSaveCallback(const F4SE::SerializationInterface* a_intfc);
    void                           OnLoadCallback(const F4SE::SerializationInterface* a_intfc);
    void                           OnRevertCallback(const F4SE::SerializationInterface* a_intfc);

    void                           F4SEMessageHandler(F4SE::MessagingInterface::Message* msg);

    class PerkState : public REX::Singleton<PerkState>
    {
    public:
        void SetState(RE::PlayerCharacter* player) { SetState((std::uint16_t) player->GetLevel(), player->perkCount); }

        void SetState(const std::uint16_t& level, const std::int8_t& perkCount)
        {
            REX::INFO("Saving PlayerCharacter state: Level {}, perk count {}", level, perkCount);
            _level     = level;
            _perkCount = perkCount;
        }

        auto GetState() const { return std::make_pair(_level, _perkCount); }

        void SetPerkProgress(const float& progress)
        {
            REX::INFO("Saving perk progress: {}", progress);
            _perkProgress = progress;
        }

        const float& GetPerkProgress() const { return _perkProgress; }

    private:
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
