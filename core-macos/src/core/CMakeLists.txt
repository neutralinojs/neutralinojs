include_directories(include)

file(GLOB SOURCES *.cpp)

include_directories(../../lib)
add_library(core ${SOURCES})