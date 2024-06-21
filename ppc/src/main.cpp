#include <fmt/format.h>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <F4SE/API.h>
#include <F4SE/Interfaces.h>
#include <F4SE/Logger.h>
#include <F4SE/Version.h>

#include "config.h"
#include "event.h"

extern "C" DLLEXPORT constinit auto F4SEPlugin_Version = []() noexcept {
    F4SE::PluginVersionData data{};

    data.PluginVersion(PPC::version);
    data.PluginName(Version::PROJECT.data());
    data.AuthorName("r52");
    data.UsesAddressLibrary(true);
    data.UsesSigScanning(false);
    data.IsLayoutDependent(true);
    data.HasNoStructUse(false);
    data.CompatibleVersions({F4SE::RUNTIME_LATEST, F4SE::RUNTIME_1_10_980, F4SE::RUNTIME_1_10_163});

    return data;
}();

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
    auto path = logger::log_directory();
    if (!path)
    {
        return false;
    }

    *path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
    auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

    auto log  = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
    log->set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::debug);
#else
    log->set_level(spdlog::level::info);
    log->flush_on(spdlog::level::warn);
#endif

    spdlog::set_default_logger(std::move(log));
    spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

    logger::info("{} v{}", Version::PROJECT, Version::NAME);

    if (a_f4se->IsEditor())
    {
        logger::critical("loaded in editor");
        return false;
    }

    const auto ver = a_f4se->RuntimeVersion();
    if (ver < F4SE::RUNTIME_1_10_163)
    {
        logger::critical("unsupported runtime v{}", ver.string());
        return false;
    }

    F4SE::Init(a_f4se);

    // Load config
    PPC::Config::GetSingleton()->LoadConfig();

    auto serialization = F4SE::GetSerializationInterface();
    serialization->SetUniqueID(PPC::id);
    serialization->SetSaveCallback(PPC::OnSaveCallback);
    serialization->SetLoadCallback(PPC::OnLoadCallback);
    serialization->SetRevertCallback(PPC::OnRevertCallback);

    auto messaging = F4SE::GetMessagingInterface();
    messaging->RegisterListener(PPC::F4SEMessageHandler);

    return true;
}
