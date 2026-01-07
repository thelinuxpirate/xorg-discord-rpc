#ifndef RPC_H
#define RPC_H

#include <string>
#include <memory>

#include "discord.h"
#include "user.h"

struct ResolvedPresence {
    std::string name;
    std::string large;
    std::string small;
};

std::string resolveAppKey(
    const PresenceConfig &cfg,
    const std::string &instance,
    const std::string &className
);

ResolvedPresence resolvePresence(
    const PresenceConfig &cfg,
    const std::string &instance,
    const std::string &windowTitle,
    const std::string &windowClass
);

static bool ensureDiscordCore(
    std::unique_ptr<discord::Core> &core,
    const int64_t APP_ID
);

int runDiscordPresence(
    const int64_t APP_ID,
    bool daemon,
    const PresenceConfig &cfg
);

#endif
