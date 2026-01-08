# In order to build: download Discord-Game-SDK (Legacy: v2.5.6) & follow \
# https://discord.com/developers/docs/developer-tools/game-sdk#game-sdk instructions
cmake_minimum_required(VERSION 3.10)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)

project(xorg-discord-rpc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(DISCORD_SDK_DIR "" CACHE PATH "Path to Discord Game SDK")

if (DISCORD_SDK_DIR STREQUAL "")
    message(FATAL_ERROR
        "DISCORD_SDK_DIR is not set.\n"
        "Example:\n"
        "  cmake -DDISCORD_SDK_DIR=/opt/discord-sdk .."
    )
endif()

if (NOT EXISTS "${DISCORD_SDK_DIR}/include/discord.h")
    message(FATAL_ERROR
        "Invalid Discord Game SDK path:\n"
        "  ${DISCORD_SDK_DIR}\n"
        "Expected include/discord.h"
    )
endif()

find_package(X11 REQUIRED)

include(FetchContent)

FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(cli11_proj)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

set(APP_SOURCES
    src/main.cpp
    src/xTools.cpp
    src/daemon.cpp
    src/rpc.cpp
    src/user.cpp
    src/misc.cpp
)

set(DISCORD_SDK_SOURCES
    ${DISCORD_SDK_DIR}/src/core.cpp
    ${DISCORD_SDK_DIR}/src/activity_manager.cpp
    ${DISCORD_SDK_DIR}/src/application_manager.cpp
    ${DISCORD_SDK_DIR}/src/achievement_manager.cpp
    ${DISCORD_SDK_DIR}/src/image_manager.cpp
    ${DISCORD_SDK_DIR}/src/lobby_manager.cpp
    ${DISCORD_SDK_DIR}/src/network_manager.cpp
    ${DISCORD_SDK_DIR}/src/overlay_manager.cpp
    ${DISCORD_SDK_DIR}/src/storage_manager.cpp
    ${DISCORD_SDK_DIR}/src/store_manager.cpp
    ${DISCORD_SDK_DIR}/src/types.cpp
    ${DISCORD_SDK_DIR}/src/relationship_manager.cpp
    ${DISCORD_SDK_DIR}/src/user_manager.cpp
    ${DISCORD_SDK_DIR}/src/voice_manager.cpp
)

add_executable(xorg-discord-rpc
    ${APP_SOURCES}
    ${DISCORD_SDK_SOURCES}
)

target_include_directories(xorg-discord-rpc PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${DISCORD_SDK_DIR}/include
    ${X11_INCLUDE_DIR}
)

target_link_directories(xorg-discord-rpc PRIVATE
    ${DISCORD_SDK_DIR}/lib
)

target_link_libraries(xorg-discord-rpc PRIVATE
    dl
    pthread
    discord_game_sdk
    CLI11::CLI11
    tomlplusplus::tomlplusplus
    ${X11_LIBRARIES}
)

install(TARGETS xorg-discord-rpc
    RUNTIME DESTINATION bin
)

install(FILES
    README.md
    CONFIGURATION.md
    DESTINATION share/doc/xorg-discord-presence
)
