//#include <iostream>
//#include <string>

#include <unordered_map>
#include <filesystem>
#include <toml++/toml.hpp>

#include "user.h"

using namespace std;
namespace fs = filesystem;

using RuleTable = unordered_map<
    string,
    unordered_map<string, string>
>;

UserConfig getUserConfig() {
    const char* xdg = getenv("XDG_CONFIG_HOME");
    const char* home = getenv("HOME");

    fs::path base =
        xdg ? xdg :
        home ? fs::path(home) / ".config" :
        "/etc/xdg";

    fs::path dir = base / "dwm-presence";

    return {
        dir.string(),
        (dir / "config.toml").string()
    };
}

bool generateConfig(const UserConfig &cfg) {
    fs::create_directories(cfg.configDir);

    if (fs::exists(cfg.configFile)) {
        return true;
    } else {

    }

    ofstream out(cfg.configFile);
    if (!out) return false;

    out <<
        R"(# dwm-presence default config

        [example]
        default_large_image = "archlinux"
        default_small_image = ""
        alt_large_image = "discord"
        alt_small_image = "gnu"
)";

    return true;
}

RuleTable loadRules(const string &file) {
    RuleTable rules;

    auto tbl = toml::parse_file(file);

    if (auto apps = tbl["apps"].as_table()) {
        for (auto&& [appName, appNode] : *apps) {
            if (!appNode.is_table()) continue;

            auto& rule = rules[string(appName.str())];

            for (auto&& [key, val] : *appNode.as_table()) {
                if (auto s = val.value<string>()) {
                    rule[string(key.str())] = *s;
                }
            }
        }
    }

    return rules;
}
