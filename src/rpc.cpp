#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <filesystem>

#include "discord.h"
#include "rpc.h"
#include "daemon.h"
#include "user.h"
#include "misc.h"
#include "xTools.h"

#include <X11/Xlib.h>

using namespace std;
namespace fs = filesystem;

string resolveAppKey(
    const PresenceConfig &cfg,
    const string &instance,
    const string &className
) {
    if (auto it = cfg.aliases.find(instance); it != cfg.aliases.end())
        return it->second;

    if (auto it = cfg.aliases.find(className); it != cfg.aliases.end())
        return it->second;

    return instance != "unknown" ? instance : className;
}

ResolvedPresence resolvePresence(
    const PresenceConfig &cfg,
    const string &instance,
    const string &className,
    const string &windowTitle
) {
    string appKey = resolveAppKey(cfg, instance, className);

    if (windowTitle == "unknown") {
        return {
            cfg.empty.name.empty() ? windowTitle : cfg.empty.name,
            cfg.empty.large.empty() ? cfg.defaults.large : cfg.empty.large,
            cfg.empty.small.empty() ? cfg.defaults.small : cfg.empty.small
        };
    }

    if (auto it = cfg.apps.find(appKey); it != cfg.apps.end()) {
        const auto &r = it->second;
        string detailsName =
            !r.name.empty() ? r.name :
            instance != "unknown" ? instance :
            className;

        return {
            detailsName,
            r.large.empty() ? cfg.defaults.large : r.large,
            r.small.empty() ? cfg.defaults.small : r.small
        };
    }

    string detailsFallback;
    switch (cfg.settings.details) {
        case DetailsSource::Title:
            detailsFallback = windowTitle;
            break;
        case DetailsSource::App:
            detailsFallback = instance != "unknown" ? instance : className;
            break;
        case DetailsSource::Class:
            detailsFallback = className;
            break;
    }

    return {
        detailsFallback,
        cfg.defaults.large,
        cfg.defaults.small
    };
}

static bool ensureDiscordCore(
    unique_ptr<discord::Core> &core,
    const int64_t APP_ID
) {
    if (core) {
        return true;
    }

    discord::Core* raw{};
    auto res = discord::Core::Create(
        APP_ID,
        DiscordCreateFlags_NoRequireDiscord,
        &raw
    );

    if (res == discord::Result::Ok && raw) {
        core.reset(raw);
        return true;
    }

    return false;
}

int runDiscordPresence(
    const int64_t APP_ID,
    bool daemon,
    const PresenceConfig &cfg
) {
    pid_t pid;
    if (isDaemonRunning(pid)) {
        cerr << "[rpc] daemon already running (PID " << pid << ")\n";
        return 1;
    }

    Display* dpy = XOpenDisplay(nullptr);
    if (!dpy) {
        cerr << "[rpc] cannot open X display\n";
        return 1;
    }

    XSetIOErrorHandler([](Display*) -> int {
        fs::remove(getPidFilePath());
        _exit(0);
    });

    if (daemon) {
        daemonize();
    }

    installSignals();
    auto appPid = getPidFilePath();
    ofstream(appPid) << getpid();

    string wmTitle = "unkown";
    for (int i = 0; i < 50; i++) { // 5 second wait
        wmTitle = getWindowManagerName(dpy);
        if (!wmTitle.empty() && wmTitle != "unknown")
            break;

        usleep(100000);
    }

    unique_ptr<discord::Core> stateCore{nullptr};
    discord::Activity activity{};

    activity.SetType(discord::ActivityType::Playing);
    activity.GetTimestamps().SetStart(time(nullptr));

    string lastInstance, lastClass, lastTitle;
    auto lastCheck = chrono::steady_clock::now();

    while (!interrupted.load(memory_order_relaxed)) {
        if (!ensureDiscordCore(stateCore, APP_ID)) {
            this_thread::sleep_for(chrono::seconds(2));
            continue;
        }

        stateCore->RunCallbacks();
        auto now = chrono::steady_clock::now();

        if (now - lastCheck >= chrono::milliseconds(500)) {
            lastCheck = now;

            auto wc = getWindowClass(dpy);

            string inst = normalize(wc.instance);
            string cls = normalize(wc.className);
            string title = getWindowTitle(dpy);

            if (inst != lastInstance
                || cls != lastClass
                || title != lastTitle
            ) {
                lastInstance = inst;
                lastClass = cls;
                lastTitle = title;

                auto resolved = resolvePresence(cfg, inst, cls, title);

                string details;
                switch (cfg.settings.details) {
                    case DetailsSource::App:
                        details = resolved.name;
                        break;
                    case DetailsSource::Title:
                        details = title;
                        break;
                    case DetailsSource::Class:
                    default:
                        details = cls;
                        break;
                }

                details = capitalizeFirstLetter(details);
                activity.SetDetails(details.c_str());

                if (cfg.settings.state != StateSource::Empty) {
                    switch (cfg.settings.state) {
                        case StateSource::Title:
                            activity.SetState(title.c_str());
                            break;
                        case StateSource::Class:
                            activity.SetState(cls.c_str());
                            break;
                        case StateSource::App:
                            activity.SetState(resolved.name.c_str());
                            break;
                        default:
                            break;
                    }
                }

                // large image
                if (resolved.large == "wm") {
                    activity.GetAssets().SetLargeImage(sanitize_asset(wmTitle).c_str());
                } else {
                    activity.GetAssets().SetLargeImage(resolved.large.c_str());
                }

                activity.GetAssets().SetLargeText(sanitize_asset(wmTitle).c_str());

                // small image
                activity.GetAssets().SetSmallImage(resolved.small.c_str());
                activity.GetAssets().SetSmallText(title.c_str());

                stateCore->ActivityManager().UpdateActivity(
                    activity,
                    [&](discord::Result result) {
                        if (result != discord::Result::Ok) {
                            cerr << "[rpc] Lost Discord connection\n";
                            stateCore.reset();
                        }
                    }
        );
            }
        }

        this_thread::sleep_for(chrono::milliseconds(16));
    }

    fs::remove(appPid);
    return 0;
}
