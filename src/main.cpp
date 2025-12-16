#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <cassert>
#include <csignal>
#include <cstdio>
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
    auto result = discord::Core::Create(310270644849737729, DiscordCreateFlags_Default, &core); // opens application??
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

        state.core->UserManager().GetUser(130050050968518656,
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

    state.core->ActivityManager().RegisterCommand("run/command/foo/bar/baz/here.exe"); // ?
    state.core->ActivityManager().RegisterSteam(123123321); // Steam Game???

    state.core->ActivityManager().OnActivityJoin.Connect(
      [](const char* secret) { std::cout << "Join " << secret << "\n"; });
    state.core->ActivityManager().OnActivitySpectate.Connect(
      [](const char* secret) { std::cout << "Spectate " << secret << "\n"; });
    state.core->ActivityManager().OnActivityJoinRequest.Connect([](discord::User const& user) {
        std::cout << "Join Request " << user.GetUsername() << "\n";
    });
    state.core->ActivityManager().OnActivityInvite.Connect(
      [](discord::ActivityActionType, discord::User const& user, discord::Activity const&) {
          std::cout << "Invite " << user.GetUsername() << "\n";
      });

    state.core->LobbyManager().OnLobbyUpdate.Connect(
      [](std::int64_t lobbyId) { std::cout << "Lobby update " << lobbyId << "\n"; });

    state.core->LobbyManager().OnLobbyDelete.Connect(
      [](std::int64_t lobbyId, std::uint32_t reason) {
          std::cout << "Lobby delete " << lobbyId << " (reason: " << reason << ")\n";
      });

    state.core->LobbyManager().OnMemberConnect.Connect(
      [](std::int64_t lobbyId, std::int64_t userId) {
          std::cout << "Lobby member connect " << lobbyId << " userId " << userId << "\n";
      });

    state.core->LobbyManager().OnMemberUpdate.Connect(
      [](std::int64_t lobbyId, std::int64_t userId) {
          std::cout << "Lobby member update " << lobbyId << " userId " << userId << "\n";
      });

    state.core->LobbyManager().OnMemberDisconnect.Connect(
      [](std::int64_t lobbyId, std::int64_t userId) {
          std::cout << "Lobby member disconnect " << lobbyId << " userId " << userId << "\n";
      });

    state.core->LobbyManager().OnLobbyMessage.Connect([&](std::int64_t lobbyId,
                                                          std::int64_t userId,
                                                          std::uint8_t* payload,
                                                          std::uint32_t payloadLength) {
        std::vector<uint8_t> buffer{};
        buffer.resize(payloadLength);
        memcpy(buffer.data(), payload, payloadLength);
        std::cout << "Lobby message " << lobbyId << " from " << userId << " of length "
                  << payloadLength << " bytes.\n";

        char fourtyNinetySix[4096];
        state.core->LobbyManager().GetLobbyMetadataValue(lobbyId, "foo", fourtyNinetySix);

        std::cout << "Metadata for key foo is " << fourtyNinetySix << "\n";
    });

    state.core->LobbyManager().OnSpeaking.Connect(
      [&](std::int64_t, std::int64_t userId, bool speaking) {
          std::cout << "User " << userId << " is " << (speaking ? "" : "NOT ") << "speaking.\n";
      });

    discord::Activity activity{};
    activity.SetDetails("Fruit Tarts"); // smaller desc
    activity.SetState("Pop Snacks"); // app state
    activity.GetAssets().SetSmallImage("the");
    activity.GetAssets().SetSmallText("i mage");
    activity.GetAssets().SetLargeImage("the");
    activity.GetAssets().SetLargeText("u mage");
    activity.GetSecrets().SetJoin("join secret");
    activity.GetParty().GetSize().SetCurrentSize(1);
    activity.GetParty().GetSize().SetMaxSize(5);
    activity.GetParty().SetId("party id");
    activity.SetType(discord::ActivityType::Playing);
    state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cout << ((result == discord::Result::Ok) ? "Succeeded" : "Failed")
                  << " updating activity!\n";
    });

    discord::LobbyTransaction lobby{};
    state.core->LobbyManager().GetLobbyCreateTransaction(&lobby);
    lobby.SetCapacity(2);
    lobby.SetMetadata("foo", "bar");
    lobby.SetMetadata("baz", "bat");
    lobby.SetType(discord::LobbyType::Public);
    state.core->LobbyManager().CreateLobby(
      lobby, [&state](discord::Result result, discord::Lobby const& lobby) {
          if (result == discord::Result::Ok) {
              std::cout << "Created lobby with secret " << lobby.GetSecret() << "\n";
              std::array<uint8_t, 234> data{};
              state.core->LobbyManager().SendLobbyMessage(
                lobby.GetId(),
                reinterpret_cast<uint8_t*>(data.data()),
                static_cast<uint32_t>(data.size()),
                [](discord::Result result) {
                    std::cout << "Sent message. Result: " << static_cast<int>(result) << "\n";
                });
          }
          else {
              std::cout << "Failed creating lobby. (err " << static_cast<int>(result) << ")\n";
          }

          discord::LobbySearchQuery query{};
          state.core->LobbyManager().GetSearchQuery(&query);
          query.Limit(1);
          state.core->LobbyManager().Search(query, [&state](discord::Result result) {
              if (result == discord::Result::Ok) {
                  std::int32_t lobbyCount{};
                  state.core->LobbyManager().LobbyCount(&lobbyCount);
                  std::cout << "Lobby search succeeded with " << lobbyCount << " lobbies.\n";
                  for (auto i = 0; i < lobbyCount; ++i) {
                      discord::LobbyId lobbyId{};
                      state.core->LobbyManager().GetLobbyId(i, &lobbyId);
                      std::cout << "  " << lobbyId << "\n";
                  }
              }
              else {
                  std::cout << "Lobby search failed. (err " << static_cast<int>(result) << ")\n";
              }
          });
      });

    state.core->RelationshipManager().OnRefresh.Connect([&]() {
        std::cout << "Relationships refreshed!\n";

        state.core->RelationshipManager().Filter(
          [](discord::Relationship const& relationship) -> bool {
              return relationship.GetType() == discord::RelationshipType::Friend;
          });

        std::int32_t friendCount{0};
        state.core->RelationshipManager().Count(&friendCount);

        state.core->RelationshipManager().Filter(
          [&](discord::Relationship const& relationship) -> bool {
              return relationship.GetType() == discord::RelationshipType::Friend &&
                relationship.GetUser().GetId() < state.currentUser.GetId();
          });

        std::int32_t filteredCount{0};
        state.core->RelationshipManager().Count(&filteredCount);

        discord::Relationship relationship{};
        for (auto i = 0; i < filteredCount; ++i) {
            state.core->RelationshipManager().GetAt(i, &relationship);
            std::cout << relationship.GetUser().GetId() << " "
                      << relationship.GetUser().GetUsername() << "#"
                      << relationship.GetUser().GetDiscriminator() << "\n";
        }
    });

    state.core->RelationshipManager().OnRelationshipUpdate.Connect(
      [](discord::Relationship const& relationship) {
          std::cout << "Relationship with " << relationship.GetUser().GetUsername()
                    << " updated!\n";
      });

    std::signal(SIGINT, [](int) { interrupted = true; });

    do {
        state.core->RunCallbacks();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    } while (!interrupted);

    return 0;
}
