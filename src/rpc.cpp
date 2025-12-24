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
        case DetailsSource::Title: detailsFallback = windowTitle; break;
        case DetailsSource::App: detailsFallback = windowClass; break; // class is the default fallback option
        case DetailsSource::Class: detailsFallback = windowClass; break;
    }

    return {
        detailsFallback,
        cfg.defaults.large,
        cfg.defaults.small
    };
}

int runDiscordPresence(
    const long &APP_ID,
    bool daemon,
    const PresenceConfig &cfg
) {
    if (!daemon) {
        killDaemon();
        return 0;
    }

    daemonize();
    installSignals();

    discord::Core* core{};
    if (discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core) != discord::Result::Ok || !core) {
        cerr << "Failed to instantiate discord core!\n";
        return -1;
    }
    unique_ptr<discord::Core> stateCore(core);

    ofstream(PID_FILE) << getpid();

    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
    activity.GetAssets().SetLargeText(getWindowManagerName().c_str());
    activity.GetTimestamps().SetStart(time(nullptr));

    string lastClass, lastTitle;

    auto lastCheck = chrono::steady_clock::now();

    while (!interrupted.load(memory_order_relaxed)) {
        stateCore->RunCallbacks();

        auto now = chrono::steady_clock::now();
        if (now - lastCheck >= chrono::milliseconds(500)) {
            lastCheck = now;

            string winClass = getWindowClass();
            string title    = getWindowTitle();

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

         activity.SetDetails(capitalizeFirstLetter(details).c_str());

         // large image
         if (resolved.large == "wm") {
             activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
         } else {
             activity.GetAssets().SetLargeImage(resolved.large.c_str());
         }

         // small image
         activity.GetAssets().SetSmallImage(resolved.small.c_str());
         activity.GetAssets().SetSmallText(title.c_str());

         // cerr << "[resolvePresence]\n"
         //      << "  details = '" << details << "'\n"
         //      << "  resolved.name = '" << resolved.name << "'\n"
         //      << "  class = " << winClass << "\n"
         //      << "  title = " << title << "\n"
         //      << "  resolved.large = '" << resolved.large << "'\n"
         //      << "  resolved.small = '" << resolved.small << "'\n";

         stateCore->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
             cout << ((result == discord::Result::Ok) ? "Updated Status!\n" : "Status Failed!\n");
         });
            }
        }

        this_thread::sleep_for(chrono::milliseconds(16));
    }

    unlink(PID_FILE);
    return 0;
}
