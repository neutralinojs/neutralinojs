cmake_minimum_required(VERSION 3.10)
project(ExampleNativeExtension LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Build as a shared library
add_library(example_extension SHARED example_extension.cpp)

# Set output name based on platform
if(WIN32)
    set_target_properties(example_extension PROPERTIES 
        OUTPUT_NAME "example_extension"
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    set_target_properties(example_extension PROPERTIES 
        OUTPUT_NAME "example_extension"
        PREFIX "lib"
        SUFFIX ".dylib"
    )
else()
    set_target_properties(example_extension PROPERTIES 
        OUTPUT_NAME "example_extension"
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Set visibility for non-Windows platforms
if(NOT WIN32)
    target_compile_options(example_extension PRIVATE -fvisibility=hidden)
endif()

# Install to parent directory
install(TARGETS example_extension 
    LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..
    RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..
)
