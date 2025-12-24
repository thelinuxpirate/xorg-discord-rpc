#ifndef USER_H
#define USER_H

#include <unordered_map>
#include <string>

struct UserConfig {
    std::string configDir;
    std::string configFile;
};

enum class DetailsSource {
    Title,
    Class,
    App
};

struct PresenceSettings {
    DetailsSource details = DetailsSource::Title;
};

struct ImagePair {
    std::string large;
    std::string small;
};

struct AppAssets {
    std::string name;
    std::string large;
    std::string small;
};

struct PresenceConfig {
    ImagePair defaults;
    AppAssets empty;
    PresenceSettings settings;
    std::unordered_map<std::string, std::string> aliases;
    std::unordered_map<std::string, AppAssets> apps;
};

UserConfig getUserConfig();
PresenceConfig loadConfig(const std::string &path);

bool generateConfig(const UserConfig &cfg);
bool loadConfig(const UserConfig &cfg);

int setUp();

#endif
