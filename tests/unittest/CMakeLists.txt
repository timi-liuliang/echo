MESSAGE( STATUS "Configuring module: unittest")

# set module name
SET(MODULE_NAME unittest)

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty/googletest/include)

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/Fmod/Win32/Lib)
LINK_DIRECTORIES(${ECHO_DEP_PATH}/AdrenoSDK/Lib/Win32)
LINK_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/live2d/Cubism31SdkNative-EAP5/Core/lib/windows/x86)

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm *.cc)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

IF(ECHO_UNICODE)
	ADD_DEFINITIONS("-DUNICODE -D_UNICODE")
ENDIF()

# generate module library
ADD_EXECUTABLE(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# link libararies
TARGET_LINK_LIBRARIES(${MODULE_NAME} googletest engine)

TARGET_LINK_LIBRARIES(${MODULE_NAME} zlib engine 7zip winmm.lib imm32.lib dxgi.lib Shlwapi.lib lua recast freeimage)
IF(CMAKE_SIZEOF_VOID_P EQUAL 4)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} fmod_vc.lib fmodstudio_vc.lib)
ELSEIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} fmod64_vc.lib fmodstudio64_vc.lib)
ENDIF()

IF(ECHO_ARCHIVE_SUPPORT_7ZIP)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} 7zip)
ENDIF()

TARGET_LINK_LIBRARIES(${MODULE_NAME} Live2DCubismCore.lib pugixml)

# 性能分析服务
IF(ECHO_PROFILER)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} ws2_32.lib)
ENDIF()

TARGET_LINK_LIBRARIES(${MODULE_NAME} debug libEGL_d.lib optimized libEGL.lib)
TARGET_LINK_LIBRARIES(${MODULE_NAME} debug libGLESv2_d.lib optimized libGLESv2.lib dbghelp.lib Psapi.lib)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "tests")

# log
MESSAGE(STATUS "Configure success!")
