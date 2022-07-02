# infoware - C++ System information Library
#
# Written in 2016-2020 by nabijaczleweli <nabijaczleweli@gmail.com> and ThePhD <phdofthehouse@gmail.com>
#
# To the extent possible under law, the author(s) have dedicated all copyright and related
# and neighboring rights to this software to the public domain worldwide. This software is
# distributed without any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>


cmake_minimum_required(VERSION 3.6.0)


set(infoware_version "0.6.0")

project(infoware VERSION "${infoware_version}" LANGUAGES CXX)


option(INFOWARE_USE_D3D    "Add public, transitive define to infoware to use Direct 3D for GPU detection."                       OFF)
option(INFOWARE_USE_OPENGL "Add public, transitive define to infoware to use Open Graphics Language (OpenGL) for GPU detection." OFF)
option(INFOWARE_USE_OPENCL "Add public, transitive define to infoware to use Open Compute Language (OpenCL) for GPU detection."  OFF)

option(INFOWARE_USE_X11 "Add public, transitive define to infoware to use X11 for display detection." OFF)

option(INFOWARE_EXAMPLES "Add infoware examples to the build." OFF)
option(INFOWARE_TESTS    "Input tests for infoware."           OFF)

set(INFOWARE_PCI_IDS_PATH       ""                                   CACHE FILEPATH "pci.ids file to use. Overrides cloning from INFOWARE_PCI_IDS_REPOSITORY.")
set(INFOWARE_PCI_IDS_REPOSITORY "https://github.com/pciutils/pciids" CACHE STRING
		"Path/URL to clone a pciids git repository from if override path not supplied. Defaults to https://github.com/pciutils/pciids.")

if(INFOWARE_USE_D3D)
	if(NOT WIN32)
		message(FATAL_ERROR "INFOWARE_USE_D3D only available on Win32.")
	endif()
	if(INFOWARE_USE_OPENCL)
		message(WARNING "INFOWARE_USE_OPENCL specified, but the higher-priority INFOWARE_USE_D3D was specified, too, and will be used instead.")
	elseif(INFOWARE_USE_OPENGL)
		message(WARNING "INFOWARE_USE_OPENGL specified, but the higher-priority INFOWARE_USE_D3D was specified, too, and will be used instead.")
	endif()
endif()

if(INFOWARE_USE_OPENCL)
	if(INFOWARE_USE_OPENGL)
		message(WARNING "INFOWARE_USE_OPENGL specified, but the higher-priority INFOWARE_USE_OPENCL was specified, too, and will be used instead.")
	endif()
endif()

if(INFOWARE_USE_X11 AND WIN32)
	message(WARNING "INFOWARE_USE_X11 specified, but compiling for Win32, WinAPI will be used instead.")
endif()


file(GLOB_RECURSE infoware_source_files    LIST_DIRECTORIES false CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE infoware_private_headers LIST_DIRECTORIES false CONFIGURE_DEPENDS include/infoware/detail/*.hpp)
file(GLOB         infoware_public_headers  LIST_DIRECTORIES false CONFIGURE_DEPENDS include/infoware/*.hpp)

add_library(infoware ${infoware_source_files} ${infoware_public_headers} ${infoware_private_headers})
add_library(iware::infoware ALIAS infoware)


target_include_directories(infoware PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

set_target_properties(infoware PROPERTIES CXX_STANDARD 14
                                          CXX_STANDARD_REQUIRED ON
                                          CXX_EXTENSIONS OFF

                                          PUBLIC_HEADER "${infoware_public_headers}"
                                          RUNTIME_OUTPUT_DIRECTORY "bin"
                                          LIBRARY_OUTPUT_DIRECTORY "bin"
                                          ARCHIVE_OUTPUT_DIRECTORY "lib"

                                          DEBUG_POSTFIX "d"
                                          MINSIZEREL_POSTFIX "msr"
                                          RELWITHDEBINFO_POSTFIX "rwdi")

target_compile_definitions(infoware PRIVATE INFOWARE_VERSION="${infoware_version}"
                                            INFOWARE_BUILDING=1)
if(BUILD_SHARED_LIBS)  # Controls add_library() w/o explicit mode
	target_compile_definitions(infoware PUBLIC INFOWARE_DLL=1)
endif()

if(NOT Git_FOUND)  # Could be pre-injected
	find_package(Git)
endif()

set(infoware_pci_ids_error "\
The pci.ids file, downloadable from https://github.com/pciutils/pciids or http://pci-ids.ucw.cz, is required for building infoware, \
and cloned automatically from that GitHub repository by default.\n\
To use a local copy, set INFOWARE_PCI_IDS_PATH to its location.")
if(INFOWARE_PCI_IDS_PATH)
	if(NOT EXISTS "${INFOWARE_PCI_IDS_PATH}")
		message(WARNING "The specified pci.ids file INFOWARE_PCI_IDS_PATH=${INFOWARE_PCI_IDS_PATH} doesn't seem to exist.")
	endif()
	set(infoware_pci_ids_file "${INFOWARE_PCI_IDS_PATH}")
elseif(NOT Git_FOUND)
	message(SEND_ERROR "Couldn't find a usable git executable in the environment, and the CMake variable INFOWARE_PCI_IDS_PATH is empty.\n${infoware_pci_ids_error}")
else()
	# Thanks, @griwes
	set(infoware_pci_ids_file "${CMAKE_CURRENT_BINARY_DIR}/pciids/pci.ids")
	if(EXISTS "${infoware_pci_ids_file}")
		execute_process(COMMAND "${GIT_EXECUTABLE}" remote set-url origin "${INFOWARE_PCI_IDS_REPOSITORY}"
		                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pciids")
		execute_process(COMMAND "${GIT_EXECUTABLE}" pull
		                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pciids"
		                RESULT_VARIABLE infoware_git_pciids_clone_err)
	else()
		execute_process(COMMAND "${GIT_EXECUTABLE}" clone "${INFOWARE_PCI_IDS_REPOSITORY}" -- pciids
		                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		                RESULT_VARIABLE infoware_git_pciids_clone_err)
	endif()
	if(infoware_git_pciids_clone_err)
		message(SEND_ERROR "Cloning/pulling pciids repository from ${INFOWARE_PCI_IDS_REPOSITORY} failed with ${infoware_git_pciids_clone_err}.\n${infoware_pci_ids_error}")
	endif()
endif()

add_executable(infoware_pci_generator tools/pci_generator.cpp)
set_target_properties(infoware_pci_generator PROPERTIES CXX_STANDARD 14
                                                        CXX_STANDARD_REQUIRED ON
                                                        CXX_EXTENSIONS OFF)

set(INFOWARE_PCI_DATA_HPP pci_data.hpp)
set(INFOWARE_PCI_DATA_GEN "infoware_generated/${INFOWARE_PCI_DATA_HPP}")

add_custom_command(OUTPUT ${INFOWARE_PCI_DATA_GEN}
                   COMMAND ${CMAKE_COMMAND} -E make_directory infoware_generated/
                   COMMAND $<TARGET_FILE:infoware_pci_generator> "${infoware_pci_ids_file}" > "infoware_generated/pci_data.hpp"
                   DEPENDS "${infoware_pci_ids_file}"
                   COMMENT "Generating ${INFOWARE_PCI_DATA_HPP}")

add_custom_target(infoware_generate_pcis DEPENDS "${INFOWARE_PCI_DATA_GEN}")
add_dependencies(infoware infoware_generate_pcis)


if(MSVC)
	# Only CMake 3.15.0 makes this stuff not necessary, but we have a shitty
	# minimum required version :/
	string(REGEX REPLACE "/W[0-4]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	target_compile_options(infoware PRIVATE /EHsc /W4)
	target_compile_definitions(infoware PRIVATE UNICODE _UNICODE)
else()
	target_compile_options(infoware PRIVATE -pedantic -Wall -Wextra -pipe)
endif()

if(WIN32 AND NOT MSVC)
	target_compile_options(infoware PRIVATE -march=native)
endif()

if(WIN32)
	target_link_libraries(infoware PRIVATE gdi32 version Ole32 OleAut32 wbemuuid ntdll)
endif()


if(APPLE)
	target_link_libraries(infoware PRIVATE "-framework CoreFoundation" "-framework CoreGraphics")
endif()


if(INFOWARE_USE_D3D)
	target_compile_definitions(infoware PUBLIC INFOWARE_USE_D3D)
	if (WIN32)
		target_link_libraries(infoware PUBLIC dxgi)
	endif()
elseif(INFOWARE_USE_OPENCL)
	target_compile_definitions(infoware PUBLIC INFOWARE_USE_OPENCL)
	if(NOT APPLE)
		find_package(OpenCL REQUIRED)
		target_link_libraries(infoware PUBLIC OpenCL::OpenCL)
	else()
		target_link_libraries(infoware PUBLIC "-framework OpenCL")
	endif()
elseif(INFOWARE_USE_OPENGL)
	target_compile_definitions(infoware PUBLIC INFOWARE_USE_OPENGL)

	if(WIN32)
		target_link_libraries(infoware PUBLIC opengl32)
	elseif(NOT APPLE)
		find_package(OpenGL REQUIRED)
		target_link_libraries(infoware PUBLIC OpenGL::GL)
	else()
		target_link_libraries(infoware PUBLIC "-framework OpenGL")
	endif()
endif()


if(NOT APPLE AND INFOWARE_USE_X11)
	include(FindX11)
	target_compile_definitions(infoware PUBLIC INFOWARE_USE_X11)
	target_include_directories(infoware PUBLIC ${X11_INCLUDE_DIR})
	target_link_libraries(infoware PUBLIC ${X11_LIBRARIES})
	if(X11_Xrandr_FOUND)
		target_link_libraries(infoware PUBLIC ${X11_Xrandr_LIB})
	else()
		message(FATAL_ERROR "Xrandr is required for infoware")
	endif()
endif()


if(INFOWARE_EXAMPLES)
	file(GLOB infoware_example_sources LIST_DIRECTORIES false examples/*.cpp)

	foreach(infoware_example_source IN LISTS infoware_example_sources)
		string(REGEX REPLACE "([^/\\]+[/\\])*([^/\\]+)\\.cpp" "\\2" infoware_example_name "${infoware_example_source}")
		string(REPLACE "/" "" infoware_example_name "${infoware_example_name}")

		add_executable(infoware_${infoware_example_name}_example "${infoware_example_source}")
		add_executable(iware::${infoware_example_name}_example ALIAS infoware_${infoware_example_name}_example)

		target_link_libraries(infoware_${infoware_example_name}_example PUBLIC infoware)

		set_target_properties(infoware_${infoware_example_name}_example PROPERTIES CXX_STANDARD 14
		                                                                           CXX_STANDARD_REQUIRED ON
		                                                                           CXX_EXTENSIONS OFF
		                                                                           RUNTIME_OUTPUT_DIRECTORY "bin"
		                                                                           LIBRARY_OUTPUT_DIRECTORY "bin"
		                                                                           ARCHIVE_OUTPUT_DIRECTORY "lib")

		if(INFOWARE_TESTS)
			add_test(NAME example_${infoware_example_name} COMMAND infoware_${infoware_example_name}_example)
		endif()
	endforeach()
endif()


if(INFOWARE_TESTS)
	enable_testing()
endif()


# Use GNU install directories if CMAKE_INSTALL_PREFIX not specified
include(GNUInstallDirs)

# Specify where library and headers will be installed
install(TARGETS infoware
        EXPORT infowareTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/infoware)
target_include_directories(infoware PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Create infowareConfigVersion package used to specify installed package version
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/cmake/infowareConfigVersion.cmake"
                                 VERSION ${VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/infowareConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/infoware
        COMPONENT devel)

# Specifying config file that will be used to find a library using find_package().
export(TARGETS ${PROJECT_NAME}
       FILE infowareConfig.cmake)
install(EXPORT infowareTargets
        FILE infowareConfig.cmake
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/infoware)
