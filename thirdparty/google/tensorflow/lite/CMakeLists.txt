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

# Base Files
SET(TFLITE_BASE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/external_cpu_backend_context.h
	${CMAKE_CURRENT_SOURCE_DIR}/external_cpu_backend_context.cc
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter.h
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cc
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter_builder.h
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter_builder.cc
	${CMAKE_CURRENT_SOURCE_DIR}/minimal_logging.h
	${CMAKE_CURRENT_SOURCE_DIR}/minimal_logging.cc
	${CMAKE_CURRENT_SOURCE_DIR}/mutable_op_resolver.h
	${CMAKE_CURRENT_SOURCE_DIR}/mutable_op_resolver.cc
	${CMAKE_CURRENT_SOURCE_DIR}/stderr_reporter.h
	${CMAKE_CURRENT_SOURCE_DIR}/stderr_reporter.cc
	${CMAKE_CURRENT_SOURCE_DIR}/util.h
	${CMAKE_CURRENT_SOURCE_DIR}/util.cc
)

# C Files
SET(TFLITE_C_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/c/c_api.h
	${CMAKE_CURRENT_SOURCE_DIR}/c/c_api.cc
	${CMAKE_CURRENT_SOURCE_DIR}/c/c_api_types.h
	${CMAKE_CURRENT_SOURCE_DIR}/c/common.h
	${CMAKE_CURRENT_SOURCE_DIR}/c/common.c
)

# Core Files
SET(TFLITE_CORE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/core/macros.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/subgraph.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/subgraph.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/error_reporter.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/error_reporter.cc
)

# Delegate files
SET(TFLITE_DELEGATES_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/interpreter_utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/interpreter_utils.cc
)

# Kernels files
SET(TFLITE_KENELS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/activations.cc
)

# All Files
SET(ALL_FILES 
	${TFLITE_BASE_FILES}
	${TFLITE_C_FILES}
	${TFLITE_CORE_FILES}
	${TFLITE_DELEGATES_FILES}
	${TFLITE_KENELS_FILES}
)

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Definition
IF(WIN32)
	ADD_DEFINITIONS(-DNOMINMAX -DWIN64 -DWIN32_LEAN_AND_MEAN -DNOGDI)
ENDIF()

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Link
TARGET_LINK_LIBRARIES(${MODULE_NAME} clog cpuinfo gemmlowp ruy)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# Message
MESSAGE(STATUS "Configure success!")
