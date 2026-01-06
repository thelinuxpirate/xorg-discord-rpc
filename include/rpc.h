#ifndef RPC_H
#define RPC_H

#include <string>
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

int runDiscordPresence(
    const int64_t APP_ID,
    bool daemon,
    const PresenceConfig &cfg
);

#endif
