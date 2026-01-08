# Maintainer: TRONG <spektralfrogadier@gmail.com>
pkgname=xorg-discord-rpc-git
pkgver=0.r0.g0000000
pkgrel=1
pkgdesc="Discord Rich Presence client for Xorg (git version)"
arch=('x86_64')
url="https://github.com/thelinuxpirate/xorg-discord-rpc"
license=('GPL3' 'custom:Discord-Game-SDK')

depends=('libx11')
makedepends=('cmake' 'gcc' 'git' 'wget' 'unzip')

provides=('xorg-discord-rpc')
conflicts=('xorg-discord-rpc')

source=(
  "git+https://github.com/thelinuxpirate/xorg-discord-rpc.git"
  "https://dl-game-sdk.discordapp.net/2.5.6/discord_game_sdk.zip"
)

sha256sums=('SKIP' 'SKIP')

pkgver() {
  cd "$srcdir/xorg-discord-rpc"
  printf "r%s.%s" \
    "$(git rev-list --count HEAD)" \
    "$(git rev-parse --short HEAD)"
}

build() {
  cd "$srcdir/xorg-discord-rpc"

  mkdir -p build
  mkdir -p discord-sdk/{include,lib,src}

  # Discord Game SDK extraction
  unzip -q "$srcdir/discord_game_sdk.zip" -d discord_game_sdk

  mv discord_game_sdk/discord-sdk/cpp/*.h discord-sdk/include/
  mv discord_game_sdk/discord-sdk/cpp/*.cpp discord-sdk/src/
  mv discord_game_sdk/discord-sdk/lib/x86_64/discord_game_sdk.so \
     discord-sdk/lib/libdiscord_game_sdk.so

  # Patch types.h (required for C++17)
  local types_h="discord-sdk/include/types.h"
  if ! grep -q "<cstdint>" "$types_h"; then
    sed -i '1i#include <cstdint>' "$types_h"
  fi

  cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DDISCORD_SDK_DIR="$PWD/discord-sdk"

  cmake --build build
}

package() {
  cd "$srcdir/xorg-discord-rpc"

  install -Dm755 build/xorg-discord-rpc \
    "$pkgdir/usr/bin/xorg-discord-rpc"

  # Create documentation directory
  install -d "$pkgdir/usr/share/doc/xorg-discord-rpc"

  # Main README
  install -Dm644 README.md \
    "$pkgdir/usr/share/doc/xorg-discord-rpc/README.md"

  # Configuration file explanation
  install -Dm644 CONFIGURATION.md \
    "$pkgdir/usr/share/doc/xorg-discord-rpc/CONFIGURATION.md"

  # More in depth details about assets
  install -Dm644 assets/README.org \
    "$pkgdir/usr/share/doc/xorg-discord-rpc/ASSETS.org"

  # Provide default config
  install -Dm644 exampleConfig.toml \
    "$pkgdir/usr/share/doc/xorg-discord-rpc/config.toml"
}
