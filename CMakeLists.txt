cmake_minimum_required(VERSION 3.19...3.24 FATAL_ERROR)
#SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(ignoreMe "${NO_PKGCFG} ${ASAN} ${ASAN_FLAGS} ${ARCH}")
option(SUBPROJECT_BUILD "Enables submodules to build as statically internally linked libs instead of binaries/tools" OFF)
option(ASAN "Enable AddressSanatizer" OFF)
option(NO_PKGCFG "Disable pkgconfig searching of libs, use dep_root for linking" OFF)
option(TSSCHECKER_STATIC_LINK "Statically link vendored libgeneral/libfragmentzip (+ prefer static .a for deps)" ON)
option(TSSCHECKER_FULLY_STATIC "Fully static binary (-static; requires static libc/toolchain, e.g. musl)" OFF)

if(TSSCHECKER_STATIC_LINK)
	# Vendored libs must be static archives. For final binary we also want
	# static runtimes where supported (does not require fully-static glibc).
	set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
	if(NOT DEFINED CMAKE_FIND_LIBRARY_SUFFIXES AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a")
	endif()
endif()

if(NOT DEFINED VERSION_COMMIT_COUNT)
	execute_process(COMMAND git rev-list --count HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE VERSION_COMMIT_COUNT ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(NOT DEFINED VERSION_COMMIT_SHA)
	execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}" OUTPUT_VARIABLE VERSION_COMMIT_SHA ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

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
		set(PROJECT_C_FLAGS "-arch ${ARCH} -fembed-bitcode=off -pthread -mrelax-all -Wno-deprecated-declarations -Wno-int-conversion")
		if(NOT "${CMAKE_BUILD_TYPE}" MATCHES "Debug")
			set(PROJECT_C_FLAGS "-flto=thin ${PROJECT_C_FLAGS}")
		endif()
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
		set(PROJECT_C_FLAGS "-Wl,--allow-multiple-definition -pthread -mrelax-all -Wno-deprecated-declarations -Wno-int-conversion")
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	elseif("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "MSYS" OR "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows")
		set(PROJECT_C_FLAGS "-Wl,--allow-multiple-definition -pthread -fPIC -Wno-deprecated-declarations -Wno-int-conversion -DCURL_STATICLIB -Bstatic -O0")
		set(PROJECT_CXX_FLAGS "${PROJECT_C_FLAGS}")
	endif()

	project(tsschecker VERSION 0.${VERSION_COMMIT_COUNT}.0 LANGUAGES C CXX)

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

	set(CMAKE_C_FLAGS_MINSIZEREL_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_C_FLAGS_RELEASE_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE_INIT}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_C_FLAGS_DEBUG_INIT "-g -O0 ${ADDITIONAL_C_FLAGS} ${PROJECT_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG_INIT} ${ASAN_FLAGS}")

	set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE_INIT}")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG_INIT}")

	set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_CXX_FLAGS_RELEASE_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE_INIT}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-flto=thin ${ADDITIONAL_C_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -O0 ${ADDITIONAL_C_FLAGS} ${PROJECT_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG_INIT} ${ASAN_FLAGS}")

	set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT}")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT}")

	set(CMAKE_C_STANDARD 17)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	add_library(tsschecker STATIC
			tsschecker/tsschecker.c
			tsschecker/download.c
			tsschecker/common.c
			tsschecker/main.c)
else()
	project(tsschecker VERSION 0.${VERSION_COMMIT_COUNT}.0 LANGUAGES C CXX)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	add_library(tsschecker STATIC
			tsschecker/tsschecker.c
			tsschecker/download.c)
endif()

target_include_directories(tsschecker PRIVATE
		"${CMAKE_SOURCE_DIR}/dep_root/include/"
		tsschecker
		external/jssy/jssy
		external/libfragmentzip/include
		external/libgeneral/include)

# Vendored libgeneral/libfragmentzip/jssy targets (no system install needed
# for libgeneral/libfragmentzip). Must come before any linking to them.
add_subdirectory(external)

# curl/zlib are also needed directly by tsschecker (download.c) (in addition
# to via libfragmentzip). Resolve them here; libfragmentzip already found
# them as well via find_package (same targets CURL::libcurl / ZLIB::ZLIB).
# OpenSSL (libcrypto) is needed on Linux/Windows for SHA in tsschecker.c
# (on macOS CommonCrypto is used instead). Resolve it here too.
if(NOT NO_PKGCFG)
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
	if(NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin" AND NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "MSYS" AND NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows")
		find_package(OpenSSL REQUIRED)
		set(TSSCHECKER_HAS_OPENSSL TRUE)
	endif()
else()
	if(EXISTS "${CMAKE_SOURCE_DIR}/dep_root")
		list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/dep_root")
	endif()
	find_package(CURL REQUIRED)
	find_package(ZLIB REQUIRED)
	if(NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin" AND NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "MSYS" AND NOT "${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Windows")
		find_package(OpenSSL REQUIRED)
		set(TSSCHECKER_HAS_OPENSSL TRUE)
	endif()
endif()

if(NOT NO_PKGCFG)
	find_package(PkgConfig REQUIRED FATAL_ERROR)
	# NOTE: libgeneral/libfragmentzip are vendored (external/), so they are
	# no longer required via pkg-config. Only remaining system deps are
	# listed here. libcurl/zlib are resolved via find_package above and
	# also linked into libfragmentzip.
	pkg_check_modules(DEPS REQUIRED
			libplist-2.0>=2.3.0
			libtatsu-1.0
			libirecovery-1.0)
	target_include_directories(tsschecker PRIVATE "${DEPS_INCLUDE_DIRS}")
	target_link_libraries(tsschecker PRIVATE
			${DEPS_LINK_LIBRARIES}
			fragmentzip
			general
			jssy
			CURL::libcurl
			ZLIB::ZLIB)
	if(TSSCHECKER_HAS_OPENSSL)
		target_link_libraries(tsschecker PRIVATE OpenSSL::Crypto)
	endif()
else()
	target_link_directories(tsschecker PRIVATE
			"${CMAKE_SOURCE_DIR}/dep_root/lib/")
	target_link_libraries(tsschecker PRIVATE
			curl
			plist-2.0
			tatsu
			irecovery-1.0
			fragmentzip
			general
			usbmuxd-2.0
			imobiledevice-glue-1.0
			imobiledevice-1.0
			z
			jssy)
	if(TSSCHECKER_HAS_OPENSSL)
		target_link_libraries(tsschecker PRIVATE OpenSSL::Crypto)
	endif()
	if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin")
		target_link_libraries(tsschecker PRIVATE
				compression
				"-framework CoreFoundation"
				"-framework IOKit")
	endif()
endif()

add_definitions(
		-DTSSCHECKER_VERSION_COUNT="${VERSION_COMMIT_COUNT}"
		-DTSSCHECKER_VERSION_SHA="${VERSION_COMMIT_SHA}"
		-DCMAKE_BUILD=1)

if(NOT SUBPROJECT_BUILD)
	set_target_properties(tsschecker PROPERTIES OUTPUT_NAME "tsschecker.0")
	install(TARGETS tsschecker
			DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

	# Statically linked executable (default). It is built from the same
	# sources as the library target (kept separate so the libtsschecker.0
	# archive layout is preserved), and links the vendored static
	# libfragmentzip/libgeneral archives.
	add_executable(tsschecker_bin
			tsschecker/tsschecker.c
			tsschecker/download.c
			tsschecker/common.c
			tsschecker/main.c)
	set_target_properties(tsschecker_bin PROPERTIES OUTPUT_NAME "tsschecker")
	target_include_directories(tsschecker_bin PRIVATE
			"${CMAKE_SOURCE_DIR}/dep_root/include/"
			tsschecker
			external/jssy/jssy
			external/libfragmentzip/include
			external/libgeneral/include)

	if(NOT NO_PKGCFG)
		target_include_directories(tsschecker_bin PRIVATE "${DEPS_INCLUDE_DIRS}")
		target_link_libraries(tsschecker_bin PRIVATE
				${DEPS_LINK_LIBRARIES}
				fragmentzip
				general
				jssy
				CURL::libcurl
				ZLIB::ZLIB)
		if(TSSCHECKER_HAS_OPENSSL)
			target_link_libraries(tsschecker_bin PRIVATE OpenSSL::Crypto)
		endif()
	else()
		target_link_directories(tsschecker_bin PRIVATE
				"${CMAKE_SOURCE_DIR}/dep_root/lib/")
		target_link_libraries(tsschecker_bin PRIVATE
				curl
				plist-2.0
				tatsu
				irecovery-1.0
				fragmentzip
				general
				usbmuxd-2.0
				imobiledevice-glue-1.0
				imobiledevice-1.0
				z
				jssy)
		if(TSSCHECKER_HAS_OPENSSL)
			target_link_libraries(tsschecker_bin PRIVATE OpenSSL::Crypto)
		endif()
		if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Darwin")
			target_link_libraries(tsschecker_bin PRIVATE
					compression
					"-framework CoreFoundation"
					"-framework IOKit")
		endif()
	endif()

	# Static runtime linking (does not force fully-static libc on glibc).
	# Full `-static` is opt-in via -DTSSCHECKER_FULLY_STATIC=ON and needs
	# a static toolchain (e.g. musl, as used in CI).
	if(TSSCHECKER_STATIC_LINK AND NOT TSSCHECKER_FULLY_STATIC)
		if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
			target_link_options(tsschecker_bin PRIVATE -static-libgcc -static-libstdc++)
			target_link_options(tsschecker PRIVATE -static-libgcc -static-libstdc++)
		endif()
	endif()
	if(TSSCHECKER_FULLY_STATIC)
		if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux")
			target_link_options(tsschecker_bin PRIVATE -static)
			# Also force static search for find_package deps on next configure.
			set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" CACHE STRING "" FORCE)
		endif()
	endif()

	install(TARGETS tsschecker_bin
			DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
else()
	add_definitions(-DTSSCHECKER_NOMAIN=1)
endif()

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
