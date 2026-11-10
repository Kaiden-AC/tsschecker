cmake_minimum_required(VERSION 3.19...3.24 FATAL_ERROR)
set(ignoreMe "${NO_PKGCFG} ${ASAN} ${ASAN_FLAGS} ${ARCH}")
option(SUBPROJECT_BUILD "Enables submodules to build as statically internally linked libs instead of binaries/tools" OFF)
option(ASAN "Enable AddressSanatizer" OFF)
option(NO_PKGCFG "Disable pkgconfig searching of libs, use dep_root for linking" OFF)
option(TSSCHECKER_STATIC_LINK "Statically link vendored libs (and prefer static .a for deps)" ON)
execute_process(COMMAND git rev-list --count HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE JSSY_VERSION_COMMIT_COUNT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT SUBPROJECT_BUILD)
	if(ARCH STREQUAL "")
		set(ARCH "${CMAKE_SYSTEM_PROCESSOR}")
	endif()

	if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin")
		if((NOT DEFINED MINVER) AND NO_PKGCFG)
			if("${ARCH}" STREQUAL "x86_64")
				set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
			else()
				set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0)
			endif()
		endif()
		if(DEFINED MINVER)
			set(CMAKE_OSX_DEPLOYMENT_TARGET "${MINVER}")
		endif()
		set(PROJECT_C_FLAGS "-arch ${ARCH} -fembed-bitcode=off -flto=thin -pthread -mrelax-all -Wno-deprecated-declarations -Wno-int-conversion")
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
		set(PROJECT_C_FLAGS "-Wl,--allow-multiple-definition -pthread -mrelax-all -Wno-deprecated-declarations -Wno-int-conversion")
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "MSYS" OR "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows")
		set(PROJECT_C_FLAGS "-Wl,--allow-multiple-definition -pthread -fPIC -Wno-deprecated-declarations -Wno-int-conversion -DCURL_STATICLIB -Bstatic -O0")
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	endif()

	project(jssy VERSION 0.${JSSY_VERSION_COMMIT_COUNT}.0 LANGUAGES C CXX)

	if(ASAN)
		set(ASAN ON)
		set(ASAN_FLAGS "-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer ${ASAN_FLAGS}")
		set(CMAKE_C_LINK_LIBRARY_FLAG "${CMAKE_C_LINK_LIBRARY_FLAG} ${ASAN_FLAGS}")
		set(CMAKE_MACOSX_RPATH 1)
		set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)
		set(CMAKE_BUILD_RPATH
				"/usr/local/lib"
				"/usr/lib"
				"/opt/procursus/lib"
				"/Library/Developer/CommandLineTools/usr/lib/clang/14.0.0/lib/darwin"
				"/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/"
				"/Applications/Xcode_14.1.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/"
				"/Applications/Xcode_14.2.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/14.0.0/lib/darwin/")
		#hardcoded because actions never updates xcode
	else()
		set(ASAN_FLAGS "")
	endif()

	set(CMAKE_C_FLAGS_MINSIZEREL_INIT "${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_C_FLAGS_RELEASE_INIT "${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE_INIT}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_C_FLAGS_DEBUG_INIT "${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG_INIT} ${ASAN_FLAGS}")

	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE_INIT}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT}")

	set(CMAKE_C_STANDARD 17)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
	project(jssy VERSION 0.${JSSY_VERSION_COMMIT_COUNT}.0 LANGUAGES C CXX)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

add_library(jssy STATIC
		jssy/jssy/jssy.c
		jssy/jssy/jssy_create.c)
target_include_directories(jssy PRIVATE
		"${CMAKE_SOURCE_DIR}/dep_root/include")
target_link_directories(jssy PRIVATE
		"${CMAKE_SOURCE_DIR}/dep_root/lib")

# ---------------------------------------------------------------------------
# libgeneral (vendored submodule: external/libgeneral)
# Upstream build is autotools-only (no CMake). We build it natively as a
# static library here so no system install (libgeneral.pc) is required.
#
# libfragmentzip only uses libgeneral's header macros.h, but we still build
# the minimal libgeneral static archive (exception.cpp, matching upstream
# --without-extras / BUILD_MINIMAL) and statically link it, as requested.
# ---------------------------------------------------------------------------

# Generate include/libgeneral/macros.h from macros.h.in.
# configure_file substitutes @LIBGENERAL_CLEANUP_IS_DISABLED@.
# The generated header is git-ignored upstream, so generating it in-tree is
# safe and is also required because libgeneral sources use:
#   #include "../include/libgeneral/macros.h" (relative to the source file).
if(NOT DEFINED LIBGENERAL_CLEANUP_IS_DISABLED)
	set(LIBGENERAL_CLEANUP_IS_DISABLED 0)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libgeneral/include/libgeneral/macros.h.in")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/libgeneral/include/libgeneral/macros.h.in"
		"${CMAKE_CURRENT_SOURCE_DIR}/libgeneral/include/libgeneral/macros.h"
		@ONLY
	)
endif()

add_library(general STATIC
		libgeneral/libgeneral/exception.cpp)
add_library(libgeneral ALIAS general)
target_include_directories(general PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/libgeneral/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/libgeneral")
set_target_properties(general PROPERTIES
	OUTPUT_NAME "general"
	POSITION_INDEPENDENT_CODE ON
	CXX_STANDARD 17
)

# ---------------------------------------------------------------------------
# libfragmentzip (vendored submodule: external/libfragmentzip)
# Upstream build is autotools-only. Build its single C source as a static
# library linked against libcurl + zlib (found via find_package, not via an
# installed libfragmentzip.pc).
# ---------------------------------------------------------------------------

if(NO_PKGCFG)
	# Look into dep_root for curl/zlib instead of system pkg-config.
	if(EXISTS "${CMAKE_SOURCE_DIR}/dep_root")
		list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/dep_root")
	endif()
endif()

# Pin pkg-config to static libs when requested, if the .pc files agree.
# (find_package(CURL/ZLIB) honor CMAKE_FIND_LIBRARY_SUFFIXES; we prefer .a.)
if(TSSCHECKER_STATIC_LINK)
	set(_OLD_SUFFIXES "${CMAKE_FIND_LIBRARY_SUFFIXES}")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.lib;.so;.dylib")
endif()

find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

if(_OLD_SUFFIXES)
	set(CMAKE_FIND_LIBRARY_SUFFIXES "${_OLD_SUFFIXES}")
	unset(_OLD_SUFFIXES)
endif()

add_library(fragmentzip STATIC
		libfragmentzip/libfragmentzip/libfragmentzip.c)
add_library(libfragmentzip ALIAS fragmentzip)
target_include_directories(fragmentzip PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/libfragmentzip/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/libgeneral/include"
)
target_link_directories(fragmentzip PRIVATE
	"${CMAKE_SOURCE_DIR}/dep_root/lib")
target_link_libraries(fragmentzip PUBLIC
	general
	CURL::libcurl
	ZLIB::ZLIB
)
set_target_properties(fragmentzip PROPERTIES
	OUTPUT_NAME "fragmentzip"
	POSITION_INDEPENDENT_CODE ON
)
