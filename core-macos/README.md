### Neutralino server for macOS

Forked from [https://github.com/yibo141/Servant](https://github.com/yibo141/Servant).

## Build Requirenments

- Xcode Command Line Tools
- CMake 3.15+

## Debug Builds

Debug builds are unoptimized fat binaries containing debug information. This is the type you should use for any debugging purpose.

- `mkdir build`
- `cd build`
- `cmake ..`
- `make -j 4`

The resulting binary will be placed in **build** directory

## Release Builds

Release builds are optimized binaries with a very small footprint. Suitable for distributions

- `mkdir release`
- `cd release`
- `cmake .. -DCMAKE_BUILD_TYPE=Release`
- `make -j 4`

The resulting binary will be placed in **release** directory

To install resulting binary to `bin` directory use

- `make install`
