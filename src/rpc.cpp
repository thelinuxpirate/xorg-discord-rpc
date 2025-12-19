// TODO: replace cout via syslog()
#define _CRT_SECURE_NO_WARNINGS
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <unistd.h>

#include "discord.h"
#include "daemon.h"
#include "user.h"
#include "xTools.h"

using namespace std;

int runDiscordPresence(const long &APP_ID, bool daemon) {
    // rerun 'signal' twice because they reset after pid forks sometimes
    if (!daemon) {
        killDaemon();
        return 0;
    }

    daemonize();
    installSignals();

   // signal(SIGTERM, handleSignal);
   // signal(SIGINT, handleSignal);

    discord::Core* core{};
    auto result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (!core) {
        cerr << "Failed to instantiate discord core!\n";
        return -1;
    }
    unique_ptr<discord::Core> stateCore(core);

    ofstream(PID_FILE) << getpid();

   // signal(SIGTERM, handleSignal);
   // signal(SIGINT, handleSignal);

    // setup activity once
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
    activity.GetAssets().SetLargeText(getWindowManagerName().c_str());
    activity.GetTimestamps().SetStart(time(nullptr));

    string lastTitle;
    string lastName;

    auto lastCheck = chrono::steady_clock::now();

    // runs Discord callbacks every 16ms, checks window every 500ms
    while (!interrupted.load(std::memory_order_relaxed)) {
        stateCore->RunCallbacks();

        auto now = chrono::steady_clock::now();
        if (now - lastCheck >= chrono::milliseconds(500)) {
            lastCheck = now;

            string title = getWindowTitle();
            string name  = getWindowName();

            if (title != lastTitle || name != lastName) {
                lastTitle = title;
                lastName  = name;

                // TODO: remove
                if (title == "unknown" || name == "unknown") {
                    title = "empty workspace";
                    name = "empty workspace";
                } else if (title == "org.wezfurlong.wezterm") {
                    title = "wezterm";
                    name = "wezterm";
                }

                string asset = sanitize_asset(title);

                activity.SetDetails(title.c_str());
                activity.GetAssets().SetSmallImage(asset.c_str());
                activity.GetAssets().SetSmallText(name.c_str());

                /*cout << endl
                          << "Current Window = "
                          << to_lower(title).c_str()
                          << endl;

                cout <<  "Current WinName = "
                          << to_lower(name).c_str()
                          << endl
                          << endl;*/

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
