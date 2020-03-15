cmake_minimum_required (VERSION 3.10)
project(NeutralinoJS)

if (NOT CMAKE_BUILD_PLATFORM)
  message(FATAL_ERROR "\nCMake don't know which platform to target!\nConsider using cmake -DCMAKE_BUILD_PLATFORM=<your-platform ..\nPlease try again.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(PTHREAD pthread)
set(LINK_OPTS ${PTHREAD})
include_directories(shared shared/include)

if(CMAKE_BUILD_PLATFORM STREQUAL "linux")
  message(STATUS "Configuring Linux platform build..")
  set(FRAMEWORK_FLAG -DWEBVIEW_GTK=1)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)
  
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
    linux/core/storage/*.cpp
  )

elseif(CMAKE_BUILD_PLATFORM STREQUAL "macos")
  message(STATUS "Configuring MacOSX platform build..")
  set(FRAMEWORK_FLAG -DWEBVIEW_COCOA=1)
  find_library(WEBKIT Webkit)

  file(GLOB SOURCES
    macos/*.cpp
    macos/cloud/*.cpp
    macos/ping/*.cpp
    macos/auth/*.cpp
    macos/core/app/*.cpp
    macos/core/computer/*.cpp
    macos/core/debug/*.cpp
    macos/core/filesystem/*.cpp
    macos/core/os/*.cpp
    macos/core/storage/*.cpp
  )
endif()

add_executable(neutralino ${SOURCES})
target_include_directories(neutralino PUBLIC core-shared)
target_include_directories(neutralino PUBLIC ${WEBKIT_INCLUDE_DIRS})

target_link_libraries(neutralino stdc++fs)
target_link_libraries(neutralino ${WEBKIT_LIBRARIES})
target_link_libraries(neutralino ${LINK_OPTS})

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})