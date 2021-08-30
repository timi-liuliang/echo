# Set Module Name
SET(MODULE_NAME tensorflow-lite)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# Include Directories
INCLUDE_DIRECTORIES(${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google)
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/eigen)
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/flatbuffers/include)
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/gemmlowp)
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/ruy)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

SET(TFLITE_C_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/c/c_api.h
	${CMAKE_CURRENT_SOURCE_DIR}/c/c_api.cc
)

# Delegate files
SET(TFLITE_KENELS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/activations.cc
)

SET(TFLITE_DELEGATES_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/gpu/api.cc
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/gpu/delegate.cc
	${CMAKE_CURRENT_SOURCE_DIR}/experimental/acceleration/compatibility/android_info.cc
)

# All Files
SET(ALL_FILES ${TFLITE_C_FILES} ${TFLITE_KENELS_FILES})

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Definition
IF(WIN32)
	ADD_DEFINITIONS(-DTFL_STATIC_LIBRARY_BUILD)
	ADD_DEFINITIONS(-DNOMINMAX -DWIN64 -DWIN32_LEAN_AND_MEAN -DNOGDI)
	ADD_DEFINITIONS(/bigobj /nologo /EHsc /GF /MP /Gm- /wd4800 /wd4805 /wd4244)
ENDIF()

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Link
TARGET_LINK_LIBRARIES(${MODULE_NAME} clog cpuinfo gemmlowp ruy)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# Message
MESSAGE(STATUS "Configure success!")
