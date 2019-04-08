MESSAGE( STATUS "Configuring module: Engine...")

# Set Module Name
SET(MODULE_NAME engine)

# Set Module Path
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# set include files
SET(INCLUDE_FILES
	${ECHO_ROOT_PATH}
	${ECHO_ROOT_PATH}/engine
	${ECHO_ROOT_PATH}/engine/core/render
	${ECHO_ROOT_PATH}/engine/core/render/render
	${ECHO_ROOT_PATH}/engine/core/render/gles
	${ECHO_ROOT_PATH}/engine/core/render/gles/RenderTask
	${ECHO_ROOT_PATH}/engine/core/render/gles/GPUProxy
	${ECHO_ROOT_PATH}/engine/core/render/gles/Android
	${ECHO_ROOT_PATH}/engine/core/script/lua
	${ECHO_ROOT_PATH}/thirdparty/
	${ECHO_ROOT_PATH}/thirdparty/spine/
	${ECHO_SRC_PATH}/ThirdParty/FreeImage
	${ECHO_ROOT_PATH}/ThirdParty/Recast/Recast
	${ECHO_ROOT_PATH}/ThirdParty/Recast/Detour
	${ECHO_ROOT_PATH}/ThirdParty/Recast/DetourCrowd
	${ECHO_ROOT_PATH}/ThirdParty/Recast/DetourTileCache
	${ECHO_ROOT_PATH}/thirdparty/opengles/mali/include
	${ECHO_ROOT_PATH}/thirdparty/physx/physx/include
	${ECHO_ROOT_PATH}/thirdparty/physx/pxshared/include
	${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/include
    ${ECHO_ROOT_PATH}/thirdparty/freetype-2.10.0/include
    ${ECHO_ROOT_PATH}/thirdparty/radeonrays/RadeonRays/include
)

IF(${ECHO_PLATFORM_WIN32})
	LIST(APPEND INCLUDE_FILES ${ECHO_ROOT_PATH}/thirdparty/openal-1.1/windows/include)
ENDIF()

INCLUDE_DIRECTORIES(${INCLUDE_FILES})

# Net
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/Dependencies/theoraplayer/SDK/include)

# Configure FILE
CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/core/base/echo_config.h.in
	${CMAKE_CURRENT_SOURCE_DIR}/core/base/echo_config.h
)

IF(${ECHO_PLATFORM_MAC})
    SET(MAC_MM_FILES *.mm)
ELSE()
    SET(MAC_MM_FILES)
ENDIF()

# Get All Files
FILE(GLOB_RECURSE HEADER_FILES  *.h *.hpp *.inl)
FILE(GLOB_RECURSE SOURCE_FILES *.cpp *.c *.cc *.cx ${MAC_MM_FILES})

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES} CMakeLists.txt)

# Remove render library
IF(${ECHO_PLATFORM_WIN32})
	EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${MODULE_PATH}/core/render/metal/*.*)
ELSEIF(${ECHO_PLATFORM_MAC})
    EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${MODULE_PATH}/core/render/gles/*.*)
	EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${MODULE_PATH}/core/render/vulkan/*.*)
ENDIF()

GROUP_FILES_NO_INCLUDE(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

IF(ECHO_UNICODE)
	ADD_DEFINITIONS("-DUNICODE -D_UNICODE")
ENDIF()

ADD_LIBRARY(${MODULE_NAME} STATIC ${ALL_FILES} CMakeLists.txt)

IF(ECHO_BUILD_PLATFORM_ANDROID)
#	android_ndk_gdb_debuggable(Engine)
  SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES VC_MDD_ANDROID_PLATFORM_TOOLSET "Clang_3_6")
  SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES VC_MDD_ANDROID_USE_OF_STL "gnustl_shared")
  SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES VC_MDD_ANDROID_API_LEVEL  "android-19")

  SET_PROPERTY(TARGET ${MODULE_NAME} PROPERTY C_STANDARD 11)
  SET_PROPERTY(TARGET ${MODULE_NAME} PROPERTY CXX_STANDARD 14)
  SET_PROPERTY(TARGET ${MODULE_NAME} PROPERTY VC_MDD_ANDROID_EXCEPTION_HANDLING "Enabled")
  SET_PROPERTY(TARGET ${MODULE_NAME} PROPERTY VC_MDD_ANDROID_HAVE_RTTI "true")
ENDIF()

IF(ECHO_PLATFORM_HTML5)
	# enable c++11, memory growth
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ELSE()
	SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES LOCATION ${ECHO_LIB_PATH})
ENDIF()

SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "core")

IF(APPLE)
  SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
ENDIF()

MESSAGE(STATUS "Configure success!")
