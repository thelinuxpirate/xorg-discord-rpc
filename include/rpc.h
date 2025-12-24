#ifndef RPC_H
#define RPC_H

#include <string>
#include "user.h"

struct ResolvedPresence {
    std::string name;
    std::string large;
    std::string small;
};

ResolvedPresence resolvePresence(
    const PresenceConfig &cfg,
    const std::string &windowTitle,
    const std::string &windowClass
);

int runDiscordPresence(const long &APP_ID, bool daemon);

#endif
