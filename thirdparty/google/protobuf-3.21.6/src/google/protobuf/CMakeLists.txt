MESSAGE(STATUS "configuring module: baseserver...")

# 附加包含目录
SET(INCLUDE_FILES
	${NET_SRC_PATH}
	${NET_SRC_PATH}/protobuf-3.21.6/src
)

INCLUDE_DIRECTORIES( ${INCLUDE_FILES})

# 链接库
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# 设置项目路径
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# 递归获取所有c++文件
SET(HEADER_FILES
)

SET(SOURCE_FILES
	${MODULE_PATH}/any_lite.cc
	${MODULE_PATH}/arena.cc
	${MODULE_PATH}/arenastring.cc
	${MODULE_PATH}/arenaz_sampler.cc
	${MODULE_PATH}/stubs/bytestream.cc
	${MODULE_PATH}/io/coded_stream.cc
	${MODULE_PATH}/stubs/common.cc
	${MODULE_PATH}/extension_set.cc
	${MODULE_PATH}/generated_enum_util.cc
	${MODULE_PATH}/generated_message_tctable_lite.cc
	${MODULE_PATH}/generated_message_util.cc
	${MODULE_PATH}/implicit_weak_message.cc
	${MODULE_PATH}/inlined_string_field.cc
	${MODULE_PATH}/stubs/int128.cc
	${MODULE_PATH}/io/io_win32.cc
	${MODULE_PATH}/map.cc
	${MODULE_PATH}/message_lite.cc
	${MODULE_PATH}/parse_context.cc
	${MODULE_PATH}/repeated_field.cc
	${MODULE_PATH}/repeated_ptr_field.cc
	${MODULE_PATH}/stubs/status.cc
	${MODULE_PATH}/stubs/statusor.cc
	${MODULE_PATH}/stubs/stringpiece.cc
	${MODULE_PATH}/stubs/stringprintf.cc
	${MODULE_PATH}/io/strtod.cc
	${MODULE_PATH}/stubs/structurally_valid.cc
	${MODULE_PATH}/stubs/strutil.cc
	${MODULE_PATH}/stubs/time.cc
	${MODULE_PATH}/wire_format_lite.cc
	${MODULE_PATH}/io/zero_copy_stream.cc
	${MODULE_PATH}/io/zero_copy_stream_impl.cc
	${MODULE_PATH}/io/zero_copy_stream_impl_lite.cc
)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# 分组文件
#GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# 预定义宏
ADD_DEFINITIONS(-D_BASE_SERVER_)

# 设置模块名称
SET(MODULE_NAME protobuf-lite)

# 新加库
ADD_LIBRARY(${MODULE_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} CMakeLists.txt)

# 设置目标属性
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

MESSAGE(STATUS "baseserver configure success!")


