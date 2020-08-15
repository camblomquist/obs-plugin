project(audio-hook VERSION 0.1.0 DESCRIPTION "OBS Audio Hook")

configure_file(
	audio-hook.rc.in
	audio-hook.rc)

configure_file(
	audio-hook-ver.hpp.in
	audio-hook-ver.hpp)

set(PROJECT_SOURCES
	funchook/funchook.c
	audio-hook.cpp
	hook-main.cpp)

set(PROJECT_HEADERS
	${CMAKE_SOURCE_DIR}/src/helpers/hook-helper.hpp
	audio-hook-info.hpp
	${CMAKE_CURRENT_BINARY_DIR}/audio-hook-ver.hpp
	funchook/funchook.hpp
	funchook/funchook.h
	audio-hook.hpp)
	
if(MSVC)
	add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
endif()

add_library(${PROJECT_NAME} MODULE 
	${CMAKE_CURRENT_BINARY_DIR}/audio-hook.rc
	${PROJECT_SOURCES}
	${PROJECT_HEADERS})
	
target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/src/helpers
	PRIVATE funchook)
	
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_output_suffix "64")
else()
	set(_output_suffix "32")
endif()

set_target_properties(${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME "${PROJECT_NAME}${_output_suffix}")
		
install_plugin_bin_to_data(${CMAKE_PROJECT_NAME} ${PROJECT_NAME})