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
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/farmhash/src)
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty/google/fp16/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories 
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Base Files
SET(TFLITE_BASE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/allocation.h
	${CMAKE_CURRENT_SOURCE_DIR}/allocation.cc
	${CMAKE_CURRENT_SOURCE_DIR}/arena_planner.h
	${CMAKE_CURRENT_SOURCE_DIR}/arena_planner.cc
	${CMAKE_CURRENT_SOURCE_DIR}/create_op_resolver.h
	${CMAKE_CURRENT_SOURCE_DIR}/create_op_resolver_with_builtin_ops.cc
	${CMAKE_CURRENT_SOURCE_DIR}/external_cpu_backend_context.h
	${CMAKE_CURRENT_SOURCE_DIR}/external_cpu_backend_context.cc
	${CMAKE_CURRENT_SOURCE_DIR}/graph_info.cc
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter.h
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter.cc
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter_builder.h
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter_builder.cc
	${CMAKE_CURRENT_SOURCE_DIR}/interpreter_experimental.cc
	${CMAKE_CURRENT_SOURCE_DIR}/minimal_logging.h
	${CMAKE_CURRENT_SOURCE_DIR}/minimal_logging.cc
	${CMAKE_CURRENT_SOURCE_DIR}/minimal_logging_default.cc
	${CMAKE_CURRENT_SOURCE_DIR}/model_builder.h
	${CMAKE_CURRENT_SOURCE_DIR}/model_builder.cc
	${CMAKE_CURRENT_SOURCE_DIR}/mmap_allocation_disabled.cc
	${CMAKE_CURRENT_SOURCE_DIR}/mutable_op_resolver.h
	${CMAKE_CURRENT_SOURCE_DIR}/mutable_op_resolver.cc
	${CMAKE_CURRENT_SOURCE_DIR}/optional_debug_tools.cc
	${CMAKE_CURRENT_SOURCE_DIR}/signature_runner.h
	${CMAKE_CURRENT_SOURCE_DIR}/signature_runner.cc
	${CMAKE_CURRENT_SOURCE_DIR}/simple_memory_arena.cc
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
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/flatbuffer_conversions.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/flatbuffer_conversions.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/op_resolver.h
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/op_resolver.cc
	${CMAKE_CURRENT_SOURCE_DIR}/core/api/tensor_utils.cc
)

# Delegate files
SET(TFLITE_DELEGATES_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/interpreter_utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/interpreter_utils.cc
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/nnapi/nnapi_delegate_disabled.cc
	${CMAKE_CURRENT_SOURCE_DIR}/nnapi/nnapi_implementation_disabled.cc
	${CMAKE_CURRENT_SOURCE_DIR}/delegates/xnnpack/quantization_util.cc
)

# Profiling files
SET(TFLITE_PROFILING_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/profiling/platform_profiler.h
	${CMAKE_CURRENT_SOURCE_DIR}/profiling/platform_profiler.cc
)

# Kernels files
SET(TFLITE_KENELS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/add.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/add_n.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/activations.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/arg_min_max.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/assign_variable.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/batch_matmul.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/batch_to_space_nd.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/bidirectional_sequence_lstm.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/bidirectional_sequence_rnn.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/broadcast_args.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/broadcast_to.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/call_once.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/cast.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/ceil.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/comparisons.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/complex_support.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/concatenation.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/conv.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/conv3d.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/conv3d_transpose.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/cpu_backend_context.h
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/cpu_backend_context.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/cpu_backend_gemm_eigen.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/optimized/cpu_check.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/cumsum.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/depth_to_space.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/densify.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/deprecated_backends.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/depthwise_conv.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/dequantize.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/detection_postprocess.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/div.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/elementwise.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/embedding_lookup.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/embedding_lookup_sparse.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/exp.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/expand_dims.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/fake_quant.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/fill.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/floor.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/floor_div.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/floor_mod.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/fully_connected.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/gather.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/gather_nd.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/gru_cell.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/hashtable.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/hashtable_find.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/hashtable_import.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/hashtable_lookup.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/hashtable_size.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/if.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/irfft2d.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/kernel_util.h
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/kernel_util.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/l2norm.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/local_response_norm.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/logical.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/lsh_projection.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/lstm.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/lstm_eval.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/matrix_diag.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/matrix_set_diag.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/maximum_minimum.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/mfcc.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/mfcc.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/mfcc_dct.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/mfcc_mel_filterbank.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/mirror_pad.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/mul.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/multinomial.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/neg.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/one_hot.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/optimized/neon_tensor_utils.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/non_max_suppression.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/numeric_verify.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/pack.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/pad.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/pooling.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/pooling3d.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/reference/portable_tensor_utils.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/pow.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/quantization_util.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/quantize.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/range.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/rank.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/read_variable.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/reduce.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/register.h
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/register.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/register_ref.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/reshape.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/resize_bilinear.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/resize_nearest_neighbor.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/reverse.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/reverse_sequence.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/rfft2d.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/roll.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/round.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/unidirectional_sequence_gru.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/unidirectional_sequence_lstm.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/unidirectional_sequence_rnn.cc
	${CMAKE_CURRENT_SOURCE_DIR}/kernels/internal/kernel_utils.cc
)

# Schema files
SET(TFLITE_SCHEMA_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/schema/schema_utils.cc
)

# All Files
SET(ALL_FILES 
	${TFLITE_BASE_FILES}
	${TFLITE_C_FILES}
	${TFLITE_CORE_FILES}
	${TFLITE_DELEGATES_FILES}
	${TFLITE_KENELS_FILES}
	${TFLITE_SCHEMA_FILES}
	${TFLITE_PROFILING_FILES}
)

# Group
#GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Definition
IF(WIN32)
	ADD_DEFINITIONS(-DNOMINMAX -DWIN64 -DWIN32_LEAN_AND_MEAN -DNOGDI -DTFL_STATIC_LIBRARY_BUILD -D_WINDOWS)
ENDIF()

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Link
TARGET_LINK_LIBRARIES(${MODULE_NAME} clog cpuinfo gemmlowp ruy)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# Message
MESSAGE(STATUS "Configure success!")
