#define _CRT_SECURE_NO_WARNINGS

#include <atomic>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "discord.h"
#include "rpcHelpers.h"
#include "xTools.h"

struct DiscordState {
    discord::User currentUser;

    std::unique_ptr<discord::Core> core;
};

std::atomic<bool> interrupted{false};
std::atomic<bool>* pInterrupted;
std::atomic<bool>* pRunning;

void handleSignal(int) {
    pInterrupted->store(true);
    pRunning->store(false);
}

/*
 * TODO:
 * Make a README that explains:
 * Build Setup
 * Application runs on assets
*/

int main(int, char**) {
    DiscordState state{};
    std::atomic<bool> running{true};

    pInterrupted = &interrupted;
    pRunning = &running;

    std::signal(SIGINT, handleSignal);

    discord::Core* core{};
    auto result = discord::Core::Create(1258511249499750400, DiscordCreateFlags_Default, &core);
    state.core.reset(core);
    if (!state.core) {
        std::cout << "Failed to instantiate discord core! (err " << static_cast<int>(result)
                  << ")\n";
        std::exit(-1);
    }

    state.core->SetLogHook(
      discord::LogLevel::Debug, [](discord::LogLevel level, const char* message) {
          std::cerr << "Log(" << static_cast<uint32_t>(level) << "): " << message << "\n";
      });

    core->UserManager().OnCurrentUserUpdate.Connect([&state]() {
        state.core->UserManager().GetCurrentUser(&state.currentUser);

        std::cout << "Current user updated: " << state.currentUser.GetUsername() << "#"
                  << state.currentUser.GetDiscriminator() << "\n";

        state.core->UserManager().GetUser(1258511249499750400, // dont know what this ID is for | 130050050968518656
                                          [](discord::Result result, discord::User const &user) {
                                              if (result == discord::Result::Ok) {
                                                  std::cout << "Get " << user.GetUsername() << "\n";
                                              }
                                              else {
                                                  std::cout << "Failed to get David!\n";
                                              }
                                          });

        discord::ImageHandle handle{};
        handle.SetId(state.currentUser.GetId());
        handle.SetType(discord::ImageType::User);
        handle.SetSize(256);

        state.core->ImageManager().Fetch(
          handle, true, [&state](discord::Result res, discord::ImageHandle handle) {
              if (res == discord::Result::Ok) {
                  discord::ImageDimensions dims{};
                  state.core->ImageManager().GetDimensions(handle, &dims);
                  std::cout << "Fetched " << dims.GetWidth() << "x" << dims.GetHeight()
                            << " avatar!\n";

                  std::vector<uint8_t> data;
                  data.reserve(dims.GetWidth() * dims.GetHeight() * 4);
                  uint8_t* d = data.data();
                  state.core->ImageManager().GetData(handle, d, static_cast<uint32_t>(data.size()));
              }
              else {
                  std::cout << "Failed fetching avatar. (err " << static_cast<int>(res) << ")\n";
              }
          });
    });

    // Rich Presence Options:
    discord::Activity activity{};
    activity.SetType(discord::ActivityType::Playing);

    // gets name of your Window Manager
    activity.GetAssets().SetLargeImage(getWindowManagerName().c_str());
    activity.GetAssets().SetLargeText(getWindowManagerName().c_str());

    // keeps it from start time getting messed up
    activity.GetTimestamps().SetStart(std::time(nullptr));

    // runs whenever the window changes
    std::thread monitorThread([&state, &running, &activity]() {
        monitorWindowChanges(running, [&](const std::string &title, const std::string &name) {
            activity.GetAssets().SetSmallImage(to_lower(title).c_str()); // MUST match asset title of your application
            activity.SetDetails(title.c_str()); // smaller desc
            activity.GetAssets().SetSmallText(name.c_str());

            // update Discord Rich Presence
            state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
                std::cout << ((result == discord::Result::Ok) ? "Updated!\n" : "Failed!\n");
            });
        });
    });

    while (!interrupted) {
        state.core->RunCallbacks();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (monitorThread.joinable()) {
        monitorThread.join();
    }

    do {
        state.core->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } while (!interrupted);

    return 0;
}
