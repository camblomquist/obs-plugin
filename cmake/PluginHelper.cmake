# Modeled after ObsPluginHelpers, but actually useful

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
set(RELEASE_DIR "${CMAKE_SOURCE_DIR}/release")

function(install_plugin target)
	# Probably pointless?
	set_target_properties(${target} PROPERTIES PREFIX "")
	add_custom_command(TARGET ${target} POST_BUILD
		# If config is Release or RelWithDebInfo, package release files
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>> == 1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:${target}>"
				"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")
		
		# If config is RelWithDebInfo, copy the pdb file
		COMMAND if $<CONFIG:RelWithDebInfo> == 1(
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_PDB_FILE:${target}>"
				"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")
		
		# Copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug> == 1(
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:${target}>"
				"$<TARGET_PDB_FILE:${target}>"
				"${OBS_BUILDDIR}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")

		VERBATIM
	)
endfunction()

function(install_plugin_data target data)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>> == 1(
			"${CMAKE_COMMAND}" -E copy_directory
				"${CMAKE_CURRENT_SOURCE_DIR}/${data}"
				"${RELEASE_DIR}/data/obs-plugins/${target}")
	
		COMMAND if $<CONFIG:Debug> == 1(
			"${CMAKE_COMMAND}" -E copy_directory
				"${CMAKE_CURRENT_SOURCE_DIR}/${data}"
				"${OBS_BUILDDIR}/rundir/$<CONFIG>/data/obs-plugins/${target}")

		VERBATIM
	)
endfunction()

function(install_plugin_with_data target data)
	install_plugin(${target})
	install_plugin_data(${target} ${data})
endfunction()

function(install_plugin_bin_to_data target additional_target)
	add_custom_command(TARGET ${additional_target} POST_BUILD
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>> == 1(
			"${CMAKE_COMMAND}" -E make_directory
				"${RELEASE_DIR}/data/obs-plugins/${target}")
		
		COMMAND if $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>> == 1(
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:${additional_target}>"
				"${RELEASE_DIR}/data/obs-plugins/${target}")
		
		COMMAND if $<CONFIG:Debug> == 1(
			"${CMAKE_COMMAND}" -E make_directory
				"${OBS_BUILDDIR}/rundir/$<CONFIG>/data/obs-plugins/${target}")
		
		COMMAND if $<CONFIG:Debug> == 1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:${additional_target}>"
				"${OBS_BUILDDIR}/rundir/$<CONFIG>/data/obs-plugins/${target}")

		VERBATIM
	)
endfunction()