export LD_LIBRARY_PATH=/discord-sdk/lib:$LD_LIBRARY_PATH

# Replace with your Discord Application-ID
APP_ID="1258511249499750400"

# Make sure you create the build directory & setup CMake on your own
cd build/
cmake ..
make
cd -

./build/xorg_discord_presence -r $APP_ID 
