export LD_LIBRARY_PATH=/discord-sdk/lib:$LD_LIBRARY_PATH

# Make sure you create the build directory & setup CMake on your own
cd build/
cmake ..
make
cd -

./build/xorg_discord_presence
