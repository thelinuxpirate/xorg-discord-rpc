export LD_LIBRARY_PATH=/discord-sdk/lib:$LD_LIBRARY_PATH

if [ ! -d "build" ]; then
  mkdir -p build
fi

cmake -S . -B build \
  -DDISCORD_SDK_DIR=discord-sdk

cmake --build build
