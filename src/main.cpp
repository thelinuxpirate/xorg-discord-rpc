#define _CRT_SECURE_NO_WARNINGS

#include <cassert>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#include "discord.h"

struct DiscordState {
    discord::User currentUser;

    std::unique_ptr<discord::Core> core;
};

namespace {
    volatile bool interrupted{false};
}

int main(int, char**) {
    DiscordState state{};

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
                                          [](discord::Result result, discord::User const& user) {
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

    // unusable as we are tracking Xorg data
    //state.core->ActivityManager().RegisterCommand("START_APP_COMMAND"); // ?

    discord::Activity activity{};

    activity.SetName("NAME_NAME_NAME");

    activity.GetAssets().SetLargeImage("dwm"); // Update manually to your Window Manager

    activity.GetAssets().SetSmallImage("archlinux"); // async update to current application

    activity.SetDetails("APPLICATION_WINDOW"); // smaller desc
    activity.SetState("APP_STATE"); // for some reason has a player icon next to it (app state)
    activity.GetAssets().SetSmallText("APP_TITLE");
    activity.GetAssets().SetLargeText("WINDOW_MANAGER_TITLE");
    activity.SetType(discord::ActivityType::Playing);
    state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                  << " updating activity!\n";
    });

    std::signal(SIGINT, [](int) { interrupted = true; });

    do {
        state.core->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } while (!interrupted);

    return 0;
}
