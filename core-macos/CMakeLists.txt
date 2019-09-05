# THIS IS NOT READY YET FOR CROSS-PLATFORM BUILDS

cmake_minimum_required (VERSION 3.15)
project(NeutralinoJS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(PTHREAD pthread)
find_library(WEBKIT WebKit)

set(FRAMEWORK_FLAG -DWEBVIEW_COCOA=1)
set(LINK_OPTS ${PTHREAD} ${WEBKIT})

file(GLOB SOURCES src/*.cpp)
add_executable(neutralino ${SOURCES})
target_include_directories(neutralino PUBLIC lib)

add_subdirectory(src/core)
add_subdirectory(src/auth)
add_subdirectory(src/ping)
add_subdirectory(src/cloud)
add_subdirectory(src/platform/fd)

target_link_libraries(neutralino core)
target_link_libraries(neutralino auth)
target_link_libraries(neutralino ping)
target_link_libraries(neutralino cloud)
target_link_libraries(neutralino fd)

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})
target_link_libraries(neutralino ${LINK_OPTS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    install(TARGETS neutralino DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()
