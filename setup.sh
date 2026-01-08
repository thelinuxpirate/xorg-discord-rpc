#!/usr/bin/env bash
set -euo pipefail

git clone https://github.com/thelinuxpirate/xorg-discord-rpc.git
cd xorg-discord-rpc

mkdir build
mkdir discord-sdk/include
mkdir discord-sdk/lib
mkdir discord-sdk/src

wget https://dl-game-sdk.discordapp.net/2.5.6/discord_game_sdk.zip
mkdir current/
unzip discord_game_sdk.zip -d current/discord-sdk

mv current/discord-sdk/cpp/*.h discord-sdk/include
mv current/discord-sdk/cpp/*.cpp discord-sdk/src
mv current/discord-sdk/lib/x86_64/discord_game_sdk.so discord-sdk/lib/libdiscord_game_sdk.so

rm -r current

TYPES_H="discord-sdk/include/types.h"

if ! grep -q "<cstdint>" "$TYPES_H"; then
    echo "Patching $TYPES_H to include <cstdint>..."
    sed -i '1a#include <cstdint>' "$TYPES_H"
fi

cmake -S . -B build \
  -DDISCORD_SDK_DIR=discord-sdk

cmake --build build
