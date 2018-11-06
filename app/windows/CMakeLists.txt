MESSAGE( STATUS "Configuring module: app")

# set module name
SET(MODULE_NAME app)

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/AdrenoSDK/Lib/Win32)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/lib/windows/x86)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/openal-1.1/windows/libs/Win32)
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/AdrenoSDK/Lib/x64)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/lib/windows/x86_64)
	LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/openal-1.1/windows/libs/Win64)
ENDIF()

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# generate module library
ADD_EXECUTABLE(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# link libraries
TARGET_LINK_LIBRARIES(${MODULE_NAME} zlib engine winmm.lib imm32.lib dxgi.lib Shlwapi.lib lua recast freeimage)
TARGET_LINK_LIBRARIES(${MODULE_NAME} Live2DCubismCore.lib pugixml spine box2d)
TARGET_LINK_LIBRARIES(${MODULE_NAME} debug libEGL_d.lib optimized libEGL.lib)
TARGET_LINK_LIBRARIES(${MODULE_NAME} OpenAL32.lib)
TARGET_LINK_LIBRARIES(${MODULE_NAME} debug libGLESv2_d.lib optimized libGLESv2.lib dbghelp.lib Psapi.lib)

# no cmd window
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /entry:mainCRTStartup")

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "app")

# log
MESSAGE(STATUS "Configure success!")
