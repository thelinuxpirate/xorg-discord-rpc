# CHATGPT GENERATED
# Im not confortable enough to make a CMake file of my own
cmake_minimum_required(VERSION 3.10)
project(xorg_discord_presence)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED)

set(SOURCES
    src/main.cpp
    src/rpcHelpers.cpp
    src/xTools.cpp

    discord-sdk/src/achievement_manager.cpp
    discord-sdk/src/activity_manager.cpp
    discord-sdk/src/application_manager.cpp
    discord-sdk/src/core.cpp
    discord-sdk/src/image_manager.cpp
    discord-sdk/src/lobby_manager.cpp
    discord-sdk/src/network_manager.cpp
    discord-sdk/src/overlay_manager.cpp
    discord-sdk/src/storage_manager.cpp
    discord-sdk/src/store_manager.cpp
    discord-sdk/src/types.cpp
    discord-sdk/src/relationship_manager.cpp
    discord-sdk/src/user_manager.cpp
    discord-sdk/src/voice_manager.cpp
)

include_directories(${X11_INCLUDE_DIR})

add_executable(xorg_discord_presence ${SOURCES})

target_include_directories(xorg_discord_presence PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/discord-sdk/include
)

set_target_properties(xorg_discord_presence PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/discord-sdk/lib"
)

target_link_directories(xorg_discord_presence PRIVATE
    ${CMAKE_SOURCE_DIR}/discord-sdk/lib
)

target_link_libraries(xorg_discord_presence
    dl
    pthread
    discord_game_sdk
    ${X11_LIBRARIES}
)
