cmake_minimum_required (VERSION 3.15)
project(NeutralinoJS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(LINK_LIBS "")

# Platform Specific
if(APPLE)
    message(STATUS "Apple Detected")
    set(FRAMEWORK_FLAG -DWEBVIEW_COCOA=1)
    find_library(WEBKIT WebKit)
    list(APPEND LINK_LIBS ${WEBKIT})
    find_library(COCOA Cocoa)
    list(APPEND LINK_LIBS ${COCOA})
elseif(WIN32)
    message(STATUS "Windows Detected")
    set(FRAMEWORK_FLAG -DWEBVIEW_WINAPI=1)
else()
    message(STATUS "Linux Detected")
    find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
	pkg_check_modules(WEBKIT REQUIRED webkitgtk-3.0)
    set(FRAMEWORK_FLAG -DWEBVIEW_GTK=1)
endif()

# Microsoft C++ specific
if(MSVC)
    message("Microsoft Visual C++ Detected")
else() # valid for MinGW also
    message("GNU Compatible compiler detected")
    find_library(PTHREAD pthread)
    list(APPEND LINK_LIBS ${PTHREAD})
endif()

file(GLOB SOURCES src/*.cc)
add_executable(neutralino ${SOURCES})
target_include_directories(neutralino PUBLIC thirdparty)

add_subdirectory(src/io/fs)
target_link_libraries(neutralino fs)

target_compile_options(neutralino PUBLIC ${FRAMEWORK_FLAG})
target_link_libraries(neutralino ${LINK_LIBS})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    install(TARGETS neutralino DESTINATION ${CMAKE_SOURCE_DIR}/bin)
endif()

