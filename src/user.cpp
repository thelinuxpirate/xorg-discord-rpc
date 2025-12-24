#include <string>
#include <filesystem>
#include <toml++/toml.hpp>

#include "user.h"
#include "xTools.h"
#include "misc.h"

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

    out <<
        R"(# xorg-presence default config example

# default option: wm = window manager name
[defaults]
details = "app" # can be "app", "class", or "title"
large_image = "wm"
small_image = ""

# map a window's class to an app table
[aliases]
firefox = "browser"
zen = "browser"
emacs = "editor"
wezterm = "terminal"
alacritty = "terminal"

# empty workspaces show up as 'unknown' (remove if you want the title to show)
[unknown]
name = "Empty Workspace"
small_image = "gnu"

# apps
[apps.browser]
small_image = "zen"

[apps.editor]
large_image = "emacs"
small_image = "tux"

[apps.terminal]
large_image = "archlinux"
small_image = "terminal")";

    return true;
}

PresenceConfig loadConfig(const string &path) {
    PresenceConfig cfg{};
    auto tbl = toml::parse_file(path);

    if (auto d = tbl["defaults"].as_table()) {
        if (auto v = d->get_as<string>("details")) {
            string mode = normalize(v->value_or("name"));

            if (mode == "title") {
                cfg.settings.details = DetailsSource::Title;
            } else if (mode == "app") {
                cfg.settings.details = DetailsSource::App;
            } else {
                cfg.settings.details = DetailsSource::Class;
            }
        }

        if (auto v = d->get_as<string>("small_image")) {
            cfg.defaults.small = v->value_or("");
        }

        if (auto v = d->get_as<string>("large_image")) {
            cfg.defaults.large = v->value_or("");
        }
    }

    if (auto u = tbl["unknown"].as_table()) {
        if (auto v = u->get_as<string>("name")) {
            cfg.empty.name = v->value_or("");
        }

        if (auto v = u->get_as<string>("large_image")) {
            cfg.empty.large = v->value_or(cfg.defaults.large);
        }

        if (auto v = u->get_as<string>("small_image")) {
            cfg.empty.small = v->value_or(cfg.defaults.small);
        }
    }

    if (auto a = tbl["aliases"].as_table()) {
        for (auto&& [k, v] : *a) {
            if (auto s = v.value<string>()) {
                cfg.aliases.emplace(k.str(), *s);
            }
        }
    }

    if (auto apps = tbl["apps"].as_table()) {
        for (auto&& [appName, node] : *apps) {
            if (!node.is_table()) continue;

            AppAssets asset{};
            auto& t = *node.as_table();

            if (auto v = t.get_as<string>("name"))
                asset.name = v->value_or(""); // leave empty

            if (auto v = t.get_as<string>("large_image"))
                asset.large = v->value_or(cfg.defaults.large);

            if (auto v = t.get_as<string>("small_image"))
                asset.small = v->value_or(cfg.defaults.small);

            cfg.apps.emplace(appName.str(), asset);
        }
    }

    return cfg;
}

int setUp() {
    auto cfg = getUserConfig();

    if (!generateConfig(cfg)) {
        return 1;
    }

    // can this be reloaded???
    loadConfig(cfg.configFile);
    return 0;
}
