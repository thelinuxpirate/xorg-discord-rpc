#define _CRT_SECURE_NO_WARNINGS
#include <atomic>
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>

#include "discord.h"
#include "xTools.h"

using namespace std;

atomic<bool> interrupted{false};

void handleSignal(int) { interrupted = true; }

string to_lower(const string &str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return tolower(c); });
    return result;
}

int runDiscordPresence(const long &APP_ID) {
    signal(SIGINT, handleSignal);

    discord::Core* core{};
    auto result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (!core) {
        cerr << "Failed to instantiate discord core!\n";
        return -1;
    }
    unique_ptr<discord::Core> stateCore(core);

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
    while (!interrupted) {
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

                activity.GetAssets().SetSmallImage(to_lower(title).c_str());
                activity.SetDetails(title.c_str());
                activity.GetAssets().SetSmallText(name.c_str());

                cout << endl
                          << "Current Window = "
                          << to_lower(title).c_str()
                          << endl;

                cout <<  "Current WinName = "
                          << to_lower(name).c_str()
                          << endl
                          << endl;

                stateCore->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
                    cout << ((result == discord::Result::Ok) ? "Updated Status!\n" : "Status Failed!\n");
                });
            }
        }

        this_thread::sleep_for(chrono::milliseconds(16));
    }

    return 0;
}
