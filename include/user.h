#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <string>

struct UserConfig {
    std::string configDir;
    std::string configFile;
};

struct AppAssets {
    std::string title;
    std::string large;
    std::string small;
};

struct PresenceConfig {
    AppAssets defaults;
    AppAssets empty;
    std::unordered_map<std::string, std::string> aliases;
    std::unordered_map<std::string, AppAssets> apps;
};

UserConfig getUserConfig();
PresenceConfig loadConfig(const std::string &path);

bool generateConfig(const UserConfig &cfg);
bool loadConfig(const UserConfig &cfg);

int setUp();

#endif
