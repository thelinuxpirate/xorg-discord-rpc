export LD_LIBRARY_PATH=/discord-sdk/lib:$LD_LIBRARY_PATH

if [ ! -d "build" ]; then
  mkdir -p build
fi

cd build/
cmake ..
make
cd -
