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
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.hpp *.cpp *.mm *.cc)

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

IF(ECHO_ARCHIVE_SUPPORT_7ZIP)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} 7zip)
ENDIF()

TARGET_LINK_LIBRARIES(${MODULE_NAME} Live2DCubismCore.lib pugixml)

# 性能分析服务
IF(ECHO_PROFILER)
	TARGET_LINK_LIBRARIES(${MODULE_NAME} ws2_32.lib)
ENDIF()

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "tests")

# log
MESSAGE(STATUS "Configure success!")
