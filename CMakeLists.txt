cmake_minimum_required (VERSION 3.10)
project(NeutralinoJS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
find_library(PTHREAD pthread)
pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)

set(FRAMEWORK_FLAG -DWEBVIEW_GTK=1)
set(LINK_OPTS ${PTHREAD} ${WEBKIT})

include_directories(core-shared)
include_directories(core-shared/include)

file(GLOB SOURCES
  linux/*.cpp
  linux/cloud/*.cpp
  linux/ping/*.cpp
  linux/auth/*.cpp
  linux/core/app/*.cpp
  linux/core/computer/*.cpp
  linux/core/debug/*.cpp
  linux/core/filesystem/*.cpp
  linux/core/os/*.cpp
  linux/core/storage/*.cpp)
add_executable(neutralino ${SOURCES})

target_link_libraries(neutralino core)
target_link_libraries(neutralino auth)
target_link_libraries(neutralino ping)
target_link_libraries(neutralino cloud)

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})
target_link_libraries(neutralino ${LINK_OPTS})
