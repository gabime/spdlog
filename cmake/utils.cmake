# Get spdlog version from include/spdlog/version.h and put it in SPDLOG_VERSION
function(spdlog_extract_version)
	file(READ "${CMAKE_CURRENT_LIST_DIR}/include/spdlog/version.h" file_contents)
	string(REGEX MATCH "SPDLOG_VER_MAJOR ([0-9]+)" _  "${file_contents}")
	set(ver_major ${CMAKE_MATCH_1})

	string(REGEX MATCH "SPDLOG_VER_MINOR ([0-9]+)" _  "${file_contents}")
	set(ver_minor ${CMAKE_MATCH_1})
	string(REGEX MATCH "SPDLOG_VER_PATCH ([0-9]+)" _  "${file_contents}")
	set(ver_patch ${CMAKE_MATCH_1})

	if (NOT ver_major OR NOT ver_minor OR NOT ver_patch)
		message(FATAL_ERROR "Could not extract valid version from spdlog/version.h")
	endif()
	set (SPDLOG_VERSION "${ver_major}.${ver_minor}.${ver_patch}" PARENT_SCOPE)    
endfunction()


# Turn on warnings on the given target 
function(spdlog_enable_warnings target_name)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
		target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wconversion -pedantic -Wfatal-errors)
	endif()
	if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		target_compile_options(${target_name} PRIVATE /W4 /WX )
	endif()	
endfunction()


# Enable address sanitizer (gcc/clang only)
function(spdlog_enable_sanitizer target_name)	
	if (NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		message(FATAL_ERROR "Sanitizer supported only for gcc/clang")
	endif()
	message(STATUS "Address sanitizer enabled")		
	target_compile_options(${target_name} "-fsanitize=address,undefined")
	target_compile_options(${target_name} "-fno-sanitize=signed-integer-overflow")	
	target_compile_options(${target_name} "-fno-sanitize-recover=all")
	target_compile_options(${target_name} "-fno-omit-frame-pointer")
	target_link_libraries(${target_name} "-fsanitize=address,undefined -fuse-ld=gold")
endfunction()