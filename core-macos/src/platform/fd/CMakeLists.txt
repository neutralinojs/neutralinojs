include_directories(../include)

if(APPLE)
    message(STATUS "Apple detected")
    set(SOURCES fd-macosx.m)
endif()

add_library(fd STATIC ${SOURCES})
target_link_libraries(fd INTERFACE "-framework Cocoa")