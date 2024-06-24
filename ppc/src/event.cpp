#include "event.h"
#include "config.h"
#include "F4SE/Interfaces.h"
#include "F4SE/Logger.h"
#include "RE/Bethesda/PlayerCharacter.h"
#include <cmath>
#include <ranges>

PPC::PerkPointEventSink _perkPointIncreaseSink;

namespace PPC
{

    void OnSaveCallback(const F4SE::SerializationInterface* a_intfc)
    {
        auto ver = (std::uint32_t) Version::MAJOR;

        a_intfc->OpenRecord(id, ver);

        // Write progress
        auto perkProgress = PerkState::GetSingleton()->GetPerkProgress();
        if (a_intfc->WriteRecordData(perkProgress))
        {
            logger::debug("wrote save data perkProgress = {}", perkProgress);
        }
    }

    void OnLoadCallback(const F4SE::SerializationInterface* a_intfc)
    {
        std::uint32_t type, ver, length;
        while (a_intfc->GetNextRecordInfo(type, ver, length))
        {
            if (type == id)
            {
                // Load progress
                float perkProgress;
                auto  read = a_intfc->ReadRecordData(&perkProgress, sizeof(perkProgress));
                if (read == sizeof(float))
                {
                    PerkState::GetSingleton()->SetPerkProgress(perkProgress);
                }
                else
                {
                    PerkState::GetSingleton()->SetPerkProgress(0.0f);
                }
            }
        }
    }

    void OnRevertCallback(const F4SE::SerializationInterface*)
    {
        logger::debug("clearing serialization data");
        auto perkState = PerkState::GetSingleton();
        perkState->SetPerkProgress(0.0f);
        perkState->SetState(1, 0);
    }

    void F4SEMessageHandler(F4SE::MessagingInterface::Message* msg)
    {
        switch (msg->type)
        {
            case F4SE::MessagingInterface::kGameDataReady:
                {
                    auto source = RE::PerkPointIncreaseEvent::GetEventSource();
                    logger::debug("source = {}", fmt::ptr(source));

                    if (source)
                    {
                        logger::info("BSTGlobalEvent::EventSource<PerkPointIncreaseEvent> found! Adding sink...");
                        source->RegisterSink(&_perkPointIncreaseSink);
                    }
                    else
                    {
                        logger::warn("RE::PerkPointIncreaseEvent::GetEventSource() failed. PPC will be inactive.");
                    }
                }
                break;
            case F4SE::MessagingInterface::kNewGame:
            case F4SE::MessagingInterface::kPostLoadGame:
                {
                    auto player = RE::PlayerCharacter::GetSingleton();

                    if (player)
                    {
                        PerkState::GetSingleton()->SetState(player);
                    }
                }
                break;
            default:
                break;
        }
    }

    RE::BSEventNotifyControl PerkPointEventSink::ProcessEvent(const RE::PerkPointIncreaseEvent& event, RE::BSTEventSource<RE::PerkPointIncreaseEvent>*)
    {
        logger::debug("PerkPointIncreaseEvent triggered! perkCount {}", event.perkCount);

        auto perkState            = PerkState::GetSingleton();
        auto player               = RE::PlayerCharacter::GetSingleton();
        auto config               = Config::GetSingleton();

        auto currentLevel         = (std::uint16_t) player->GetLevel();
        auto [oldlevel, oldCount] = perkState->GetState();

        if (event.perkCount > oldCount && currentLevel > oldlevel)
        {
            auto leveldiff = currentLevel - oldlevel;
            logger::debug("Level increased by {}, current level {}, player->perkCount = {}", leveldiff, currentLevel, player->perkCount);

            // Offset natural increase
            player->perkCount -= (std::int8_t) leveldiff;

            auto bound = (std::uint16_t)(currentLevel + 1);
            for (std::uint16_t lev : std::views::iota(oldlevel, bound) | std::views::drop(1))
            {
                bool matched = false;
                logger::debug("processing level {}", lev);

                for (const auto& [range, value] : config->rates)
                {
                    if (!range.inRange(lev))
                    {
                        continue;
                    }

                    matched = true;
                    logger::debug("level {} matches range({}, {})={}", lev, range.cbegin(), range.cend(), value);

                    auto currentPerkProgress = perkState->GetPerkProgress();
                    auto pointsToAddf        = currentPerkProgress + value;
                    auto pointsToAdd         = std::floor(pointsToAddf);
                    auto leftover            = pointsToAddf - pointsToAdd;

                    logger::info("Adding {} perk points for level {}, remaining progress {}", (std::uint16_t) pointsToAdd, lev, leftover);
                    player->perkCount += (std::int8_t) pointsToAdd;
                    perkState->SetPerkProgress(leftover);
                    break;
                }

                if (!matched)
                {
                    logger::info("no range matched for level {}...keeping natural perkCount increase", lev);
                    // Restore natural increase
                    player->perkCount += 1;
                }
            }

            logger::debug("New player->perkCount = {}", player->perkCount);
        }
        else if (currentLevel == oldlevel && event.perkCount < oldCount)
        {
            logger::debug("Perk spent");
        }
        // else do nothing for any other (potential edge) cases

        perkState->SetState(currentLevel, player->perkCount);

        return RE::BSEventNotifyControl::kContinue;
    }
}  // namespace PPC