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

/*
 * TODO:
 * turn into cli command + add function to disable RPC / restart
 * trim spaces function?
 * Make a README that explains:
 * Build Setup
 * Application runs on assets
*/

const long APP_ID = 1258511249499750400;

std::atomic<bool> interrupted{false};

void handleSignal(int) { interrupted = true; }

std::string to_lower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

int main() {
    std::signal(SIGINT, handleSignal);

    discord::Core* core{};
    auto result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
    if (!core) {
        std::cerr << "Failed to instantiate discord core!\n";
        return -1;
    }
    std::unique_ptr<discord::Core> stateCore(core);

    // setup activity once
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);
    activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
    activity.GetAssets().SetLargeText(getWindowManagerName().c_str());
    activity.GetTimestamps().SetStart(std::time(nullptr));

    std::string lastTitle;
    std::string lastName;

    auto lastCheck = std::chrono::steady_clock::now();

    // runs Discord callbacks every 16ms, checks window every 500ms
    while (!interrupted) {
        stateCore->RunCallbacks();

        auto now = std::chrono::steady_clock::now();
        if (now - lastCheck >= std::chrono::milliseconds(500)) {
            lastCheck = now;

            std::string title = getWindowTitle();
            std::string name  = getWindowName();

            if (title != lastTitle || name != lastName) {
                lastTitle = title;
                lastName  = name;

                if (title == "unknown" || name == "unknown") {
                    title = "empty workspace";
                    name = "empty workspace";
                }

                activity.GetAssets().SetSmallImage(to_lower(title).c_str());
                activity.SetDetails(title.c_str());
                activity.GetAssets().SetSmallText(name.c_str());

                /*std::cout << std::endl
                          << "Current Window = "
                          << to_lower(title).c_str()
                          << std::endl;

                std::cout <<  "Current WinName = "
                          << to_lower(name).c_str()
                          << std::endl
                          << std::endl;*/

                stateCore->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
                    std::cout << ((result == discord::Result::Ok) ? "Updated Status!\n" : "Status Failed!\n");
                });
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}
