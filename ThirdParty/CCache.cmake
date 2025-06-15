set(THIRD_PARTY_TARGET "ccache")
set(GIT_TAG "v4.11.2")
set(PROJ_DIR_NAME "${THIRD_PARTY_TARGET}_${GIT_TAG}")
set(THIRD_PARTY_TARGET_DIR "${THIRD_PARTY_INCLUDE_DIR}/${PROJ_DIR_NAME}")

find_program(CCACHE_PROGRAM ccache HINTS "${THIRD_PARTY_TARGET_DIR}/${THIRD_PARTY_TARGET}_install/bin")

if (CCACHE_PROGRAM)
    message(STATUS "ccache found: ${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found. Downloading and invoking cmake ..")
    build_third_party_project(
        false
        ${THIRD_PARTY_TARGET}
        https://github.com/ccache/ccache.git
        ${GIT_TAG}
        ${THIRD_PARTY_TARGET_DIR}
		"Release"
    )
endif()

if (CCACHE_PROGRAM)	
	# Copy ccache to binary_dir/cl.exe
	file(COPY_FILE
		${CCACHE_PROGRAM} ${CMAKE_BINARY_DIR}/cl.exe
		ONLY_IF_DIFFERENT)

	# By default Visual Studio generators will use /Zi which is not compatible
	# with ccache, so tell Visual Studio to use /Z7 instead.
	#message(STATUS "Setting MSVC debug information format to 'Embedded'")
	#set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
	if (MSVC)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Z7")
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Z7")
		set(CMAKE_VS_GLOBALS
			"CLToolExe=cl.exe"
			"CLToolPath=${CMAKE_BINARY_DIR}"
			"UseMultiToolTask=true"
			"DebugInformationFormat=OldStyle"
		)
	else()
		set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
	endif()

else()
    message(WARNING "ccache could not be found or built. Please ensure ccache is installed and the path is set correctly.")
endif()
