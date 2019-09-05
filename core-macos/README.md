# MacOS Build

Neutralino now supports MacOSX

## Build Requirenments

- XCode Libraries
- CMake

## Debug Builds

- `mkdir build`
- `cd build`
- `cmake ..`
- `make -j 4`

The resulting binary will be placed in build directory

## Release Builds

- `mkdir release`
- `cd release`
- `cmake .. -DCMAKE_BUILD_TYPE=Release`
- `make -j 4`

The resulting binary will be placed in release directory

To install resulting binary to bin directory use

`make install`
