cmake_minimum_required(VERSION 3.10)

project(obs-plugin VERSION 0.1.0)

set(PLUGIN_AUTHOR "Cameron Blomquist")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
message(FATAL_ERROR "This is a Windows-only plugin")
endif()

include(external/FindLibObs.cmake)
find_package(LibObs REQUIRED)

configure_file(
    src/plugin-macros.h.in
    ../src/plugin-macros.generated.h
)

configure_file(
    ci/ci_includes.cmd.in
    ../ci/ci_includes.generated.cmd
)

set(PLUGIN_SOURCES
	src/plugin-main.cpp)

set(PLUGIN_HEADERS
	src/plugin-macros.generated.h)

add_library(${CMAKE_PROJECT_NAME} MODULE ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

target_link_libraries(${CMAKE_PROJECT_NAME}
	libobs
)

# Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL when building with VS2019)
if (MSVC)
    add_definitions(/MP /d2FH4-)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH_NAME "64bit")
else()
	set(ARCH_NAME "32bit")
endif()

find_file(OBS_BUILDDIR
    NAMES "build" "build64" "build32"
    HINTS
		ENV obsPath${_lib_suffix}
		ENV obsPath
		${obsPath}
        "${LIBOBS_INCLUDE_DIR}/../"
    REQUIRED
)

# The "release" folder has a structure similar OBS' one on Windows
set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    # If config is Release or RelWithDebInfo, package release files
    COMMAND if $<CONFIG:Release>==1 OR $<CONFIG:RelWithDebInfo>==1 (
        "${CMAKE_COMMAND}" -E make_directory
            "${RELEASE_DIR}/data/obs-plugins/${CMAKE_PROJECT_NAME}"
            "${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
    )

    COMMAND if $<CONFIG:Release>==1 OR $<CONFIG:RelWithDebInfo>==1 (
        "${CMAKE_COMMAND}" -E copy_directory
            "${PROJECT_SOURCE_DIR}/data"
            "${RELEASE_DIR}/data/obs-plugins/${CMAKE_PROJECT_NAME}"
    )

    COMMAND if $<CONFIG:Release>==1 OR $<CONFIG:RelWithDebInfo>==1 (
        "${CMAKE_COMMAND}" -E copy
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
    )

    # If config is RelWithDebInfo, copy the pdb file
    COMMAND if $<CONFIG:RelWithDebInfo>==1 (
        "${CMAKE_COMMAND}" -E copy
            "$<TARGET_PDB_FILE:${CMAKE_PROJECT_NAME}>"
            "${RELEASE_DIR}/obs-plugins/${ARCH_NAME}"
    )

    # Copy to obs-studio dev environment for immediate testing
    COMMAND if $<CONFIG:Debug>==1 (
        "${CMAKE_COMMAND}" -E copy
            "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>"
            "$<TARGET_PDB_FILE:${CMAKE_PROJECT_NAME}>"
            "${OBS_BUILDDIR}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}"
    )

    COMMAND if $<CONFIG:Debug>==1 (
        "${CMAKE_COMMAND}" -E copy_directory
            "${PROJECT_SOURCE_DIR}/data"
            "${OBS_BUILDDIR}/rundir/$<CONFIG>/data/obs-plugins/${CMAKE_PROJECT_NAME}"
    )
)