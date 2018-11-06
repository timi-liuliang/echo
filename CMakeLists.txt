CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_VERBOSE_MAKEFILE ON)

INCLUDE(CMakeDependentOption)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS On)

PROJECT(echo)

# options
OPTION(ECHO_BUILD_PLATFORM_ANDROID "Android platform." FALSE)
OPTION(FREEIMAGE_SUPPORT_OPENEXR "FreeImage Support OpenEXR Format" FALSE)
OPTION(FREEIMAGE_SUPPORT_LIBRAW "FreeImage Support LibRaw Format" FALSE)
OPTION(FREEIMAGE_SUPPORT_WEBP "FreeImage Support WebP Format" FALSE)
OPTION(ECHO_SUPPORT_SPINE "2D Skeleton Spine Support" TRUE)
OPTION(MLPACK "mlpack Support" FALSE)

IF(UNIX)
	IF (APPLE)
		OPTION(ECHO_BUILD_PLATFORM_IOS "Mac iOS platform." FALSE)
	ELSE()
		OPTION(ECHO_BUILD_PLATFORM_ANDROID "Android platform." TRUE)
	ENDIF()
ENDIF()

OPTION(ECHO_EDITOR_MODE "Editor Mode" TRUE)

IF(ECHO_BUILD_PLATFORM_IOS AND ECHO_BUILD_PLATFORM_ANDROID)
	MESSAGE(FATAL_ERROR "Can only build for one platform.")
ENDIF()

IF(WIN32)
	IF(ECHO_BUILD_PLATFORM_ANDROID)
		ADD_DEFINITIONS("-DANDROID")

		SET(ECHO_PLATFORM_ANDROID TRUE)

		#notice the "recycling" of CMAKE_C_FLAGS
		#this is necessary to pick up android flags
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -fPIC")

		MESSAGE(STATUS "Platform: Android")
	ELSEIF(ECHO_BUILD_PLATFORM_HTML5)
		SET(ECHO_PLATFORM_HTML5 TRUE)

		# enable c++11, memory growth
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

		# enable memory growth
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")

		# enable exception catching
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")

	ELSE()
		SET(ECHO_PLATFORM_WIN32 TRUE)
		MESSAGE(STATUS "Platform: Windows")

		# compiler building setting
		# Flags used by the compiler during all build types.
		SET(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm1000 /EHsc /GR /MP /wd4996")
		# Flags used by the compiler during debug builds.
		SET(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
		# Flags used by the compiler during release minsize builds.
		SET(CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1 /Ob1 /D NDEBUG")
		# Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).
		SET(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /D NDEBUG")
		# Flags used by the compiler during Release with Debug Info builds.
		SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Ob1 /D NDEBUG")
		# Flags used by the compiler during all build types.
		SET(CMAKE_C_FLAGS "/DWIN32 /D_WINDOWS /W3 /Zm1000 /wd4996")
		# Flags used by the compiler during debug builds.
		SET(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd /Zi  /Ob0 /Od /RTC1")
		# Flags used by the compiler during release minsize builds.
		SET(CMAKE_C_FLAGS_MINSIZEREL "/MD /O1 /Ob1 /D NDEBUG")
		# Flags used by the compiler during release builds (/MD /Ob1 /Oi /Ot /Oy /Gs will produce slightly less optimized but smaller files).
		SET(CMAKE_C_FLAGS_RELEASE "/MD /O2 /Ob2 /D NDEBUG")
		# Flags used by the compiler during Release with Debug Info builds.
		SET(CMAKE_C_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Ob1 /D NDEBUG")

		IF(MSVC)
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast")

			# Enable intrinsics on MSVC in debug mode
			SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Oi")
			IF (CMAKE_CL_64)
				# Visual Studio bails out on debug builds in 64bit mode unless
				# this flag is set...
				SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
				SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
			ENDIF ()

		ENDIF()

		# This seems to break Xcode projects so definitely don't enable on Apple builds
		SET(CMAKE_USE_RELATIVE_PATHS true)
		SET(CMAKE_SUPPRESS_REGENERATION true)

	ENDIF()
ELSEIF(UNIX)
	IF(ECHO_BUILD_PLATFORM_ANDROID)

		SET(ECHO_PLATFORM_ANDROID TRUE)
		SET(CMAKE_CXX_STANDARD 14)
		SET(CMAKE_OSX_SYSROOT "" CACHE STRING "" FORCE)
		SET(CMAKE_OSX_ARCHITECTURES "" CACHE STRING "" FORCE)
		SET(XCODE_ATTRIBUTE_SDKROOT "" CACHE STRING "" FORCE)

		#notice the "recycling" of CMAKE_C_FLAGS
		#this is necessary to pick up android flags
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -fPIC")

		MESSAGE(STATUS "Platform: Android")

	ELSEIF(APPLE)
		INCLUDE(CMakeForceCompiler)
		CMAKE_FORCE_C_COMPILER(clang GNU)
		CMAKE_FORCE_CXX_COMPILER(clang++ GNU)
		SET(CMAKE_SIZEOF_VOID_P 4)
		SET(CMAKE_CXX_STANDARD 14)
		SET(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")

		IF(ECHO_BUILD_PLATFORM_IOS)
			SET(ECHO_PLATFORM_IOS TRUE)
			MESSAGE(STATUS "Platform: Mac iOS")
		ELSE()
			SET(ECHO_PLATFORM_OSX TRUE)
			MESSAGE(STATUS "Platform: Mac OSX")
		ENDIF()

		IF((CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX) AND NOT MINGW)
			SET(XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN "YES")

			IF(ECHO_PLATFORM_IOS)
				MESSAGE(STATUS "BUILD ECHO_PLATFORM_IOS")
        		SET(CMAKE_OSX_SYSROOT iphoneos CACHE STRING "" FORCE)
				SET(CMAKE_OSX_DEPLOYMENT_TARGET "")
				SET(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit")
				SET(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
				SET(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "6.0")
				SET(XCODE_ATTRIBUTE_SDKROOT iphoneos CACHE STRING "" FORCE)
				SET(XCODE_ATTRIBUTE_GCC_UNROLL_LOOPS "YES")
				SET(XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
				SET(XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES")
				SET(XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN "YES")
				SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.perfect.\${PRODUCT_NAME:rfc1034identifier}")
				#ADD_DEFINITIONS(-mno-thumb)
				#SET(CMAKE_OSX_ARCHITECTURES ${ARCHS_STANDARD_32_BIT})
				# armv7 only
				SET(CMAKE_OSX_ARCHITECTURES ${ARCHS_STANDARD})
				REMOVE_DEFINITIONS(-msse)
			ELSE()
				SET(XCODE_ATTRIBUTE_SDKROOT macosx CACHE STRING "" FORCE)
				SET(CMAKE_OSX_SYSROOT macosx CACHE STRING "" FORCE)
				#SET(CMAKE_OSX_DEPLOYMENT_TARGET 9.0)
				IF (NOT CMAKE_OSX_ARCHITECTURES)
					SET(CMAKE_OSX_ARCHITECTURES ${ARCHS_STANDARD})
				ENDIF()

        		SET(XCODE_ATTRIBUTE_GCC_INLINES_ARE_PRIVATE_EXTERN[arch=x86_64] "YES")
			ENDIF()

		ENDIF()

	ELSE()
		MESSAGE(STATUS "Platform: Unix/Linux")
	ENDIF()
ELSE()
	MESSAGE(STATUS "Platform: Unknown")
ENDIF()

SET(ECHO_ROOT_PATH "${PROJECT_SOURCE_DIR}")

IF(ECHO_PLATFORM_WIN32)
	IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
		SET(PLATFORM Win64)
	ELSE()
		SET(PLATFORM Win32)
	ENDIF()
ENDIF()

IF(WIN32)
	IF(ECHO_PLATFORM_ANDROID)
		SET(ECHO_SDK_PATH "${ECHO_ROOT_PATH}/SDK/Android")
		SET(ECHO_LIB_PATH "${ECHO_ROOT_PATH}/lib/Android")
	ELSE()
		SET(ECHO_SDK_PATH "${ECHO_ROOT_PATH}/SDK/Windows")
		SET(ECHO_LIB_PATH "${ECHO_ROOT_PATH}/lib/${PLATFORM}/")
	ENDIF()
ELSEIF(UNIX)
	IF(ECHO_PLATFORM_ANDROID)
		SET(ECHO_SDK_PATH "${ECHO_ROOT_PATH}/SDK/Android")
		SET(ECHO_LIB_PATH "${ECHO_ROOT_PATH}/lib/Android/")
	ELSEIF(APPLE)
		IF(ECHO_PLATFORM_IOS)
			SET(ECHO_SDK_PATH "${ECHO_ROOT_PATH}/SDK/iOS")
			SET(ECHO_LIB_PATH "${ECHO_ROOT_PATH}/lib/iOS/${CMAKE_BUILD_TYPE}")
		ELSE()
			SET(ECHO_SDK_PATH "${ECHO_ROOT_PATH}/SDK/OSX")
			SET(ECHO_LIB_PATH "${ECHO_ROOT_PATH}/lib/OSX/${CMAKE_BUILD_TYPE}")
		ENDIF()
	ELSE()
	ENDIF()
ENDIF()

IF(ECHO_EDITOR_MODE)
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ECHO_ROOT_PATH}/Bin/editor/${PLATFORM}/${CMAKE_BUILD_TYPE}")
	SET(ECHO_BIN_PATH "${ECHO_ROOT_PATH}/Bin/editor/${PLATFORM}/${CMAKE_BUILD_TYPE}")
ELSE()
	SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ECHO_ROOT_PATH}/Bin/app/${PLATFORM}/${CMAKE_BUILD_TYPE}")
	SET(ECHO_BIN_PATH "${ECHO_ROOT_PATH}/Bin/app/${PLATFORM}/${CMAKE_BUILD_TYPE}")
ENDIF()


SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${ECHO_LIB_PATH}/${CMAKE_BUILD_TYPE}")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ECHO_LIB_PATH}/${CMAKE_BUILD_TYPE}")
SET(ECHO_DEP_PATH "${ECHO_ROOT_PATH}/thirdparty")
SET(ECHO_DOC_PATH "${ECHO_ROOT_PATH}/Docs")

SET(ECHO_BIN_DEBUG "${ECHO_ROOT_PATH}/Bin/Debug/${PLATFORM}")
SET(ECHO_BIN_MINSIZEREL "${ECHO_ROOT_PATH}/Bin/MinSizeRel/${PLATFORM}")
SET(ECHO_BIN_RELEASE "${ECHO_ROOT_PATH}/Bin/Release/${PLATFORM}")
SET(ECHO_BIN_RELWITHDEBINFO "${ECHO_ROOT_PATH}/Bin/RelWithDebInfo/${PLATFORM}")
SET(ECHO_LIB_DEBUG "${ECHO_SDK_PATH}/Lib/Debug/${PLATFORM}")
SET(ECHO_LIB_MINSIZEREL "${ECHO_SDK_PATH}/Lib/MinSizeRel/${PLATFORM}")
SET(ECHO_LIB_RELEASE "${ECHO_SDK_PATH}/Lib/Release/${PLATFORM}")
SET(ECHO_LIB_RELWITHDEBINFO "${ECHO_SDK_PATH}/Lib/RelWithDebInfo/${PLATFORM}")

SET(ASSETS_OUT_DIR_CLASS_NAME "nonExistsClass")
SET(ASSETS_OUT_DIR_FUNCTION_NAME "getAssetsOutDir")
SET(ASSETS_OUT_DIR_SIGNATURE "()Ljava/lang/String;")

SET(APK_DIR_CLASS_NAME "nonExistsClass")
SET(APK_DIR_FUNCTION_NAME "getApkDir")
SET(APK_DIR_SIGNATURE "()Ljava/lang/String;")

MESSAGE(STATUS "\${CMAKE_BUILD_TYPE}: ${CMAKE_BUILD_TYPE}")

MESSAGE(STATUS "\${PROJECT_SOURCE_DIR}: ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "\${PROJECT_BINARY_DIR}: ${PROJECT_BINARY_DIR}")
MESSAGE(STATUS "\${ECHO_ROOT_PATH}: ${ECHO_ROOT_PATH}")
MESSAGE(STATUS "\${ECHO_LIB_PATH}: ${ECHO_LIB_PATH}")
MESSAGE(STATUS "\${ECHO_SDK_PATH}: ${ECHO_SDK_PATH}")
MESSAGE(STATUS "\${ECHO_BIN_PATH}: ${ECHO_BIN_PATH}")
MESSAGE(STATUS "\${ECHO_DOC_PATH}: ${ECHO_DOC_PATH}")

# MACRO for GROUP_FILES
MACRO(GROUP_FILES src_files root_path)
        SET(cur_dir ${root_path})
        FOREACH(group_files ${${src_files}})
            STRING(REGEX REPLACE ${cur_dir}/\(.*\) \\1 sgbd_fpath ${group_files})
            STRING(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            STRING(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            STRING(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            IF(sgbd_nogroup)
				SOURCE_GROUP("" FILES ${group_files})
			ELSE()
				INCLUDE_DIRECTORIES(${sgbd_group_name})
				SOURCE_GROUP(${sgbd_group_name} FILES ${group_files})
            ENDIF(sgbd_nogroup)
        ENDFOREACH(group_files)
ENDMACRO(GROUP_FILES)

# Macrod Define for GROUP_FILES
MACRO(GROUP_FILES_NO_INCLUDE src_files root_path)
        SET(cur_dir ${root_path})
        FOREACH(group_files ${${src_files}})
            STRING(REGEX REPLACE ${cur_dir}/\(.*\) \\1 sgbd_fpath ${group_files})
            STRING(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            STRING(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            STRING(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            IF(sgbd_nogroup)
				SOURCE_GROUP("" FILES ${group_files})
			ELSE()
				SOURCE_GROUP(${sgbd_group_name} FILES ${group_files})
            ENDIF(sgbd_nogroup)
        ENDFOREACH(group_files)
ENDMACRO(GROUP_FILES_NO_INCLUDE)

MACRO(EXCLUDE_FILES src_files rescure exclude_dir)
	FILE(${rescure} excludefiles  ${exclude_dir})
	FOREACH(excludefile ${excludefiles})
		LIST(REMOVE_ITEM ${src_files} ${excludefile})
	ENDFOREACH(excludefile)
ENDMACRO(EXCLUDE_FILES)

MACRO(APPEND_FILES src_files rescure append_dir)
	FILE(${rescure} excludefiles  ${append_dir})

	MESSAGE(STATUS ${append_dir})
	MESSAGE(STATUS ${excludefiles})

	SET(${${src_files}} ${${src_files}} ${excludefiles})
	MESSAGE(STATUS ${${src_files}})
ENDMACRO(APPEND_FILES)


# config vulkan
OPTION(VULKAN "is support vulkan" FALSE)
IF(VULKAN)
	INCLUDE(thirdparty/cmake-vulkan-sdk.txt)
ENDIF(VULKAN)


# Add sub directories
ADD_SUBDIRECTORY(engine)
ADD_SUBDIRECTORY(thirdparty/pugixml)
ADD_SUBDIRECTORY(thirdparty/physx)
ADD_SUBDIRECTORY(thirdparty/spine)
ADD_SUBDIRECTORY(thirdparty/recast)
ADD_SUBDIRECTORY(thirdparty/lua)
ADD_SUBDIRECTORY(thirdparty/freeimage)
ADD_SUBDIRECTORY(thirdparty/freetype-2.6)
ADD_SUBDIRECTORY(thirdparty/zlib)
ADD_SUBDIRECTORY(thirdparty/Box2D)
#ADD_SUBDIRECTORY(thirdparty/protobuf/src/google/protobuf)

# Editor mode
IF(ECHO_EDITOR_MODE)
	IF(ECHO_PLATFORM_WIN32)
		OPTION(ECHO_ARCHIVE_SUPPORT_7ZIP "Is support 7zip package" TRUE)
		IF(ECHO_ARCHIVE_SUPPORT_7ZIP)
			ADD_SUBDIRECTORY(thirdparty/7zip)
		ENDIF()
		ADD_SUBDIRECTORY("thirdparty/googletest")
		IF(MLPACK)
			ADD_SUBDIRECTORY("thirdparty/mlpack")
		ENDIF()
		ADD_SUBDIRECTORY("thirdparty/RadeonRays")
		ADD_SUBDIRECTORY("editor/echo")
		ADD_SUBDIRECTORY("tests/unittest")
	ELSEIF(ECHO_PLATFORM_OSX)
		ADD_SUBDIRECTORY("editor/echo")
	ENDIF()
ELSE()
	# All apps
	IF(ECHO_PLATFORM_WIN32)
		ADD_SUBDIRECTORY("app/windows")
	ELSEIF(ECHO_PLATFORM_IOS)
		ADD_SUBDIRECTORY("app/ios")
	ENDIF()
ENDIF()
