# ----- hwinfo ---------------------------------------------------------------------------------------------------------
add_library(hwinfo INTERFACE)
target_include_directories(hwinfo INTERFACE $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(lfreist-hwinfo::hwinfo ALIAS hwinfo)
set_target_properties(hwinfo PROPERTIES OUTPUT_NAME "hwinfo")

# === Components =======================================================================================================
# ----- BATTERY --------------------------------------------------------------------------------------------------------
if (HWINFO_BATTERY)
    set(BATTERY_SRC_FILES
            battery.cpp
            apple/battery.cpp
            linux/battery.cpp
            windows/battery.cpp

            windows/utils/wmi_wrapper.cpp
    )

    add_library(hwinfo_battery ${HWINFO_BUILD} ${BATTERY_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_battery PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_battery PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if(APPLE)
        target_link_libraries(hwinfo_battery PRIVATE "-framework IOKit" "-framework CoreFoundation")
    endif()

    target_link_libraries(hwinfo INTERFACE hwinfo_battery)

    set_target_properties(hwinfo_battery PROPERTIES OUTPUT_NAME "hwinfo_battery")

    install(TARGETS hwinfo_battery
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/battery.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
endif ()
# ______________________________________________________________________________________________________________________

# ----- CPU ------------------------------------------------------------------------------------------------------------
if (HWINFO_CPU)
    set(CPU_SRC_FILES
            cpu.cpp
            apple/cpu.cpp
            linux/cpu.cpp
            windows/cpu.cpp

            windows/utils/wmi_wrapper.cpp

            apple/monitoring/cpu.cpp
            linux/monitoring/cpu.cpp
            windows/monitoring/cpu.cpp
    )

    add_library(hwinfo_cpu ${HWINFO_BUILD} ${CPU_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_cpu PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_cpu PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

    target_link_libraries(hwinfo INTERFACE hwinfo_cpu)

    set_target_properties(hwinfo_cpu PROPERTIES OUTPUT_NAME "hwinfo_cpu")

    install(TARGETS hwinfo_cpu
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/cpu.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/monitoring/monitor.h
                  ${HWINFO_INCLUDE_DIR}/hwinfo/monitoring/cpu.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo/monitoring)
endif()
# ______________________________________________________________________________________________________________________

# ----- DISK -----------------------------------------------------------------------------------------------------------
if (HWINFO_DISK)
    set(DISK_SRC_FILES
            disk.cpp
            apple/disk.cpp
            linux/disk.cpp
            windows/disk.cpp

            windows/utils/wmi_wrapper.cpp

            apple/monitoring/disk.cpp
            linux/monitoring/disk.cpp
            windows/monitoring/disk.cpp
    )

    add_library(hwinfo_disk ${HWINFO_BUILD} ${DISK_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_disk PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_disk PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if(APPLE)
        target_link_libraries(hwinfo_disk PRIVATE "-framework IOKit" "-framework CoreFoundation")
    endif()


    target_link_libraries(hwinfo INTERFACE hwinfo_disk)

    set_target_properties(hwinfo_disk PROPERTIES OUTPUT_NAME "hwinfo_disk")

    install(TARGETS hwinfo_disk
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/disk.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/monitoring/disk.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo/monitoring)
endif ()
# ______________________________________________________________________________________________________________________

# ----- GPU ------------------------------------------------------------------------------------------------------------
if (HWINFO_GPU)
    set(GPU_SRC_FILES
            gpu.cpp
            apple/gpu.cpp
            linux/gpu.cpp
            windows/gpu.cpp

            windows/utils/wmi_wrapper.cpp

            PCIMapper.cpp
    )

    if (HWINFO_GPU_OPENCL)
        add_subdirectory(opencl)
    endif ()

    add_library(hwinfo_gpu ${HWINFO_BUILD} ${GPU_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_gpu PUBLIC -DHWINFO_EXPORTS)
    endif()
    if(HWINFO_GPU_OPENCL)
        target_compile_definitions(hwinfo_gpu PUBLIC USE_OCL)
        target_link_libraries(hwinfo_gpu PRIVATE opencl_device)
    endif ()
    target_include_directories(hwinfo_gpu PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<BUILD_INTERFACE:${HWINFO_CMAKE_BINARY_DIR}/generated> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    add_dependencies(hwinfo_gpu generate_pci_ids_header)

    target_link_libraries(hwinfo INTERFACE hwinfo_gpu)

    set_target_properties(hwinfo_gpu PROPERTIES OUTPUT_NAME "hwinfo_gpu")

    install(TARGETS hwinfo_gpu
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/gpu.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
    if(HWINFO_GPU_OPENCL)
        install(DIRECTORY ${HWINFO_INCLUDE_DIR}/hwinfo/opencl DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
    endif ()
endif ()
# ______________________________________________________________________________________________________________________

# ----- Mainboard ------------------------------------------------------------------------------------------------------
if (HWINFO_MAINBOARD)
    set(MAINBOARD_SRC_FILES
            mainboard.cpp
            apple/mainboard.cpp
            linux/mainboard.cpp
            windows/mainboard.cpp

            windows/utils/wmi_wrapper.cpp
    )

    add_library(hwinfo_mainboard ${HWINFO_BUILD} ${MAINBOARD_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_mainboard PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_mainboard PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    if(APPLE)
        target_link_libraries(hwinfo_mainboard PRIVATE "-framework IOKit" "-framework CoreFoundation")
    endif()

    target_link_libraries(hwinfo INTERFACE hwinfo_mainboard)

    set_target_properties(hwinfo_mainboard PROPERTIES OUTPUT_NAME "hwinfo_mainboard")

    install(TARGETS hwinfo_mainboard
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/mainboard.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
endif ()
# ______________________________________________________________________________________________________________________

# ----- OS -------------------------------------------------------------------------------------------------------------
if (HWINFO_OS)
    set(OS_SRC_FILES
            os.cpp
            apple/os.cpp
            linux/os.cpp
            windows/os.cpp

            windows/utils/wmi_wrapper.cpp
    )

    add_library(hwinfo_os ${HWINFO_BUILD} ${OS_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_os PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_os PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)



    target_link_libraries(hwinfo INTERFACE hwinfo_os)

    set_target_properties(hwinfo_os PROPERTIES OUTPUT_NAME "hwinfo_os")

    install(TARGETS hwinfo_os
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/os.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
endif ()
# ______________________________________________________________________________________________________________________

# ----- RAM ------------------------------------------------------------------------------------------------------------
if (HWINFO_RAM)
    set(RAM_SRC_FILES
            ram.cpp
            apple/ram.cpp
            linux/ram.cpp
            windows/ram.cpp

            windows/utils/wmi_wrapper.cpp

            apple/monitoring/ram.cpp
            linux/monitoring/ram.cpp
            windows/monitoring/ram.cpp
    )

    add_library(hwinfo_ram ${HWINFO_BUILD} ${RAM_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_ram PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_ram PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

    target_link_libraries(hwinfo INTERFACE hwinfo_ram)

    set_target_properties(hwinfo_ram PROPERTIES OUTPUT_NAME "hwinfo_ram")

    install(TARGETS hwinfo_ram
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)

    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/ram.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/monitoring/ram.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo/monitoring)
endif ()
# ______________________________________________________________________________________________________________________

# ----- NETWORK --------------------------------------------------------------------------------------------------------
if (HWINFO_NETWORK)
    set(NETWORK_SRC_FILES
            network.cpp
            apple/network.cpp
            linux/network.cpp
            windows/network.cpp

            windows/utils/wmi_wrapper.cpp
    )

    add_library(hwinfo_network ${HWINFO_BUILD} ${NETWORK_SRC_FILES})
    if(${HWINFO_SHARED})
        target_compile_definitions(hwinfo_network PUBLIC -DHWINFO_EXPORTS)
    endif()
    target_include_directories(hwinfo_network PUBLIC $<BUILD_INTERFACE:${HWINFO_INCLUDE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)



    target_link_libraries(hwinfo INTERFACE hwinfo_network)

    set_target_properties(hwinfo_network PROPERTIES OUTPUT_NAME "hwinfo_network")

    install(TARGETS hwinfo_network
            EXPORT lfreist-hwinfoTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION lib
            RUNTIME DESTINATION bin)
    install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/network.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
endif ()
# ______________________________________________________________________________________________________________________

install(FILES ${HWINFO_INCLUDE_DIR}/hwinfo/platform.h ${HWINFO_INCLUDE_DIR}/hwinfo/hwinfo.h  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
install(DIRECTORY ${HWINFO_INCLUDE_DIR}/hwinfo/utils DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwinfo)
install(TARGETS hwinfo
        EXPORT lfreist-hwinfoTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
