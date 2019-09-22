cmake_minimum_required (VERSION 3.15)
project(NeutralinoJS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LINK_LIBS "")

# Apple Specific
if(APPLE)
    message(STATUS "Using Cocoa Framework")
    set(FRAMEWORK_FLAG -DWEBVIEW_COCOA=1)
    find_library(WEBKIT WebKit)
    list(APPEND LINK_LIBS ${WEBKIT})
elseif(UNIX AND NOT APPLE)
    message(STATUS "Using GTK Framework")
    set(FRAMEWORK_FLAG -DWEBVIEW_GTK=1)
elseif(WIN32)
    message(STATUS "Using WinAPI")
    set(FRAMEWORK_FLAG -DWEBVIEW_WINAPI=1)
endif()

# Find these libraries from any *nix
if(UNIX)
    find_library(PTHREAD pthread)
    list(APPEND LINK_LIBS ${PTHREAD})
endif(UNIX)

# Microsoft C++ specific
if(MSVC)
    
endif(MSVC)


file(GLOB SOURCES src/*.cc)
add_executable(neutralino ${SOURCES})
target_include_directories(neutralino PUBLIC thirdparty)

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})
target_link_libraries(neutralino ${LINK_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    install(TARGETS neutralino DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()
