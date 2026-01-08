# Maintainer: TRONG <spektralfrogadier@gmail.com>
pkgname=xorg-discord-rpc
pkgver=1.0.0
pkgrel=1
pkgdesc="Discord Rich Presence Client for Xorg"
arch=('x86_64')
url="https://github.com/thelinuxpirate/xorg-discord-rpc"
license=('GPL3' 'custom:Discord-Game-SDK')
depends=('libx11')
makedepends=('cmake' 'gcc' 'wget' 'unzip' 'git')
source=(
  "git+https://github.com/thelinuxpirate/xorg-discord-rpc.git"
  "https://dl-game-sdk.discordapp.net/2.5.6/discord_game_sdk.zip"
)
sha256sums=('SKIP' 'SKIP')

build() {
  cd "$srcdir/$pkgname"

  # Create build directory
  if [ ! -d "build" ]; then
    mkdir -p build
  fi

  # Create missing SDK directories
  mkdir -p "$srcdir/xorg-discord-rpc/discord-sdk/"{include,lib,src}

  # Move everything to the proper build location
  mv "$srcdir/cpp/"*.h discord-sdk/include/
  mv "$srcdir/cpp/"*.cpp discord-sdk/src/
  mv "$srcdir/lib/x86_64/discord_game_sdk.so" \
     discord-sdk/lib/libdiscord_game_sdk.so

  # Patch types.h
  local types_h="discord-sdk/include/types.h"
  if ! grep -q "<cstdint>" "$types_h"; then
    sed -i '1i#include <cstdint>' "$types_h"
  fi

  # Build
  cmake -S . -B build -DDISCORD_SDK_DIR="$PWD/discord-sdk"
  cmake --build build
}

package() {
  cd "$srcdir/$pkgname"

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
