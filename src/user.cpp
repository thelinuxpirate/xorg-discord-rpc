#include <string>
#include <filesystem>
#include <toml++/toml.hpp>

#include "user.h"
#include "xTools.h"

using namespace std;
namespace fs = filesystem;

UserConfig getUserConfig() {
    const char* xdg  = getenv("XDG_CONFIG_HOME");
    const char* home = getenv("HOME");

    string wm = sanitize_asset(getWindowManagerName());

    fs::path base =
        xdg  ? fs::path(xdg) :
        home ? fs::path(home) / ".config" :
               fs::path("/etc/xdg");

    fs::path dir = base / (wm + "-presence");
    fs::create_directories(dir);

    return {
        dir.string(),
        (dir / "config.toml").string()
    };
}


bool generateConfig(const UserConfig &cfg) {
    fs::create_directories(cfg.configDir);

    if (fs::exists(cfg.configFile)) {
        return true;
    }

    ofstream out(cfg.configFile);
    if (!out) {
        return false;
    }

    // TODO: make "wm" string default to getWindowManagerName
    out <<
        R"(# xorg-presence default config example

# if your window title doesnt match an existing asset \
# set it to an existing default asset
[defaults]
small_image = ""

# here you can rename your window title to match whatever \
# your app asset is titled
[aliases]
firefox = "browser"
zen = "browser"
emacs = "editor"
wezterm = "terminal"
alacritty = "terminal"

# empty workspaces show up as 'unkown' remove if \
# you dont mind having your window title show up as 'unkown'
[unkown]
title = "Empty Workspace"
small_image = "gnu"

[apps.browser]
small_image = "zen"

# you can also set the large image instead of it defaulting \
# to your WM's logo
[apps.editor]
large_image = "emacs"
small_image = "tux"

[apps.terminal]
default_large_image = "archlinux"
default_small_image = "terminal")";

    return true;
}

PresenceConfig loadConfig(const string &path) {
    PresenceConfig cfg;
    auto tbl = toml::parse_file(path);

    if (auto d = tbl["defaults"].as_table()) {
        if (auto v = d->get_as<string>("small_image"))
            cfg.defaults.small = v->value_or("");
    }

    if (auto e = tbl["unknown"].as_table()) {
        if (auto v = e->get_as<string>("large_image"))
            cfg.empty.large = v->value_or("");

        if (auto v = e->get_as<string>("small_image"))
            cfg.empty.small = v->value_or(cfg.defaults.small);
    }

    if (auto a = tbl["aliases"].as_table()) {
        for (auto&& [k, v] : *a) {
            if (auto s = v.value<string>()) {
                cfg.aliases[std::string(k.str())] = *s;
            }
        }
    }

    if (auto apps = tbl["apps"].as_table()) {
        for (auto&& [app, node] : *apps) {
            if (!node.is_table()) continue;

            auto& entry = cfg.apps[std::string(app.str())];
            auto& t = *node.as_table();

            if (auto v = t.get_as<string>("large_image"))
                entry.large = v->value_or("");

            if (auto v = t.get_as<string>("small_image"))
                entry.small = v->value_or(cfg.defaults.small);
        }
    }

    return cfg;
}

int setUp() {
    auto cfg = getUserConfig();

    if (!generateConfig(cfg)) {
        return 1;
    }

    loadConfig(cfg.configFile);
    return 0;
}
