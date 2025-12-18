#ifndef USER_H
#define USER_H

#include <string>

struct UserConfig {
    std::string configDir;
    std::string configFile;
};

UserConfig getUserConfig();
bool generateConfig(const UserConfig &cfg);
bool loadConfig(const UserConfig &cfg);

#endif
