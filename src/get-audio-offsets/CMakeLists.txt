project(get-audio-offsets)

set(PROJECT_HEADERS
	../audio-hook-info.hpp
	get-audio-offsets.hpp)

set(PROJECT_SOURCES
	audioses-offsets.cpp
	get-audio-offsets.cpp)

if(MSVC)
	add_compile_options($<IF:$<CONFIG:Debug>,/MTd,/MT>)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS})

target_link_libraries(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${CMAKE_PROJECT_NAME})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_output_suffix "64")
else()
	set(_output_suffix "32")
endif()

set_target_properties(${PROJECT_NAME}
	PROPERTIES
		OUTPUT_NAME "${PROJECT_NAME}${_output_suffix}")
		
install_plugin_bin_to_data(${CMAKE_PROJECT_NAME} ${PROJECT_NAME})