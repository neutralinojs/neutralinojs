echo "Debug build for MacOSX"
test ! -d build && mkdir -p build
cd build || exit
cmake ..

echo "Building with max core: $(sysctl -n hw.physicalcpu)"
make -j "$(sysctl -n hw.physicalcpu)"

