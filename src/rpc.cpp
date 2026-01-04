#define _CRT_SECURE_NO_WARNINGS
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>

#include "discord.h"
#include "rpc.h"
#include "daemon.h"
#include "user.h"
#include "misc.h"
#include "xTools.h"

#include <X11/Xlib.h>

using namespace std;

ResolvedPresence resolvePresence(
    const PresenceConfig &cfg,
    const string &windowClass,
    const string &windowTitle
) {
    string normClass = normalize(windowClass);
    auto itAlias = cfg.aliases.find(normClass);
    string appKey = (itAlias != cfg.aliases.end()) ? itAlias->second : normClass;
    string detailsFallback;

    if (windowTitle == "unknown") {
        return {
            cfg.empty.name.empty() ? windowTitle : cfg.empty.name,
            cfg.empty.large.empty() ? cfg.defaults.large : cfg.empty.large,
            cfg.empty.small.empty() ? cfg.defaults.small : cfg.empty.small
        };
    }

    if (auto it = cfg.apps.find(appKey); it != cfg.apps.end()) {
        const auto &r = it->second;

        // FALLBACK: use window class if name field is null from table
        string detailsName = r.name.empty() ? windowClass : r.name;

        return {
            detailsName,
            r.large.empty() ? cfg.defaults.large : r.large,
            r.small.empty() ? cfg.defaults.small : r.small
        };
    }

    switch (cfg.settings.details) {
        case DetailsSource::Title:
            detailsFallback = windowTitle;
            break;
        case DetailsSource::App:
            detailsFallback = windowClass; // class is the default fallback option
            break;
        case DetailsSource::Class:
            detailsFallback = windowClass;
            break;
    }

    return {
        detailsFallback,
        cfg.defaults.large,
        cfg.defaults.small
    };
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
        unlink(PID_FILE);
        _exit(0);
    });

    if (daemon) {
        daemonize();
    }

    installSignals();
    ofstream(PID_FILE) << getpid();

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
    activity.GetAssets().SetLargeImage(wmTitle.c_str());
    activity.GetAssets().SetLargeText(wmTitle.c_str());
    activity.GetTimestamps().SetStart(time(nullptr));

    string lastClass, lastTitle;
    auto lastCheck = chrono::steady_clock::now();

    while (!interrupted.load(memory_order_relaxed)) {
        if (!stateCore) {
            discord::Core* core{};
            auto res = discord::Core::Create(
                APP_ID,
                DiscordCreateFlags_NoRequireDiscord,
                &core
            );

            if (res == discord::Result::Ok && core) {
                stateCore.reset(core);
            } else {
                this_thread::sleep_for(chrono::seconds(2));
                continue;
            }
        }

        stateCore->RunCallbacks();
        auto now = chrono::steady_clock::now();

        if (now - lastCheck >= chrono::milliseconds(500)) {
            lastCheck = now;

            string winClass = getWindowClass(dpy);
            string title = getWindowTitle(dpy);

            if (winClass != lastClass || title != lastTitle) {
                lastClass = winClass;
                lastTitle = title;

                auto resolved = resolvePresence(cfg, winClass, title);

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
                        details = winClass;
                        break;
                }

                details = capitalizeFirstLetter(details);
                activity.SetDetails(details.c_str());

                // large image
                if (resolved.large == "wm") {
                    activity.GetAssets().SetLargeImage(wmTitle.c_str());
                } else {
                    activity.GetAssets().SetLargeImage(resolved.large.c_str());
                }

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

    unlink(PID_FILE);
    return 0;
}
