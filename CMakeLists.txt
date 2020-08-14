cmake_minimum_required(VERSION 3.10)

project(win-audio-session-capture VERSION 0.1.0)

set(PLUGIN_AUTHOR "Cameron Blomquist")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
message(FATAL_ERROR "This is a Windows-only plugin")
endif()

include(cmake/PluginHelper.cmake)
include(cmake/FindLibObs.cmake)
find_package(LibObs REQUIRED)

configure_file(
    src/plugin-macros.hpp.in
    src/plugin-macros.hpp)

configure_file(
    ci/ci_includes.cmd.in
    ci/ci_includes.cmd)

set(PLUGIN_SOURCES
    src/helpers/audio-session-helper.cpp
    src/helpers/process-pipe.cpp
	src/helpers/windows-helper.cpp
    src/audio-capture.cpp
    src/preinit.cpp
	src/plugin-main.cpp)

set(PLUGIN_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/src/plugin-macros.hpp
    src/helpers/audio-session-helper.hpp
	src/helpers/process-pipe.hpp
    src/helpers/windows-helper.hpp
    src/audio-capture.hpp
    src/preinit.hpp)

add_library(${CMAKE_PROJECT_NAME} MODULE
	${PLUGIN_SOURCES}
	${PLUGIN_HEADERS})

target_include_directories(${CMAKE_PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src
	PRIVATE src)
	
target_link_libraries(${CMAKE_PROJECT_NAME}
	libobs)

# Enable Multicore Builds and disable FH4 (to not depend on VCRUNTIME140_1.DLL when building with VS2019)
if (MSVC)
    add_compile_options($<$<CONFIG:Debug>:/W4> /MP /d2FH4-)
endif()

install_plugin_with_data(${CMAKE_PROJECT_NAME} "data")

add_subdirectory(src/get-audio-offsets)