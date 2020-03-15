cmake_minimum_required (VERSION 3.10)
project(NeutralinoJS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(PTHREAD pthread)
set(LINK_OPTS ${PTHREAD})
include_directories(core-shared core-shared/include)

if(CMAKE_BUILD_PLATFORM STREQUAL "linux")
  message(STATUS "Configuring Linux platform build..")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)
  set(FRAMEWORK_FLAG -DWEBVIEW_GTK=1)
  
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
endif()

add_executable(neutralino ${SOURCES})
target_include_directories(neutralino PUBLIC core-shared)

target_link_libraries(neutralino stdc++fs)
target_link_libraries(neutralino ${WEBKIT_LIBRARIES})
target_include_directories(neutralino PUBLIC ${WEBKIT_INCLUDE_DIRS})

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})
target_link_libraries(neutralino ${LINK_OPTS})