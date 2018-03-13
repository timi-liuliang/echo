MESSAGE(STATUS "configuring module: baseserver...")

# 附加包含目录
SET(INCLUDE_FILES
	${ECHO_ROOT_PATH}
	${ECHO_ROOT_PATH}/thirdparty/protobuf/src
)

INCLUDE_DIRECTORIES( ${INCLUDE_FILES})

# 链接库
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
#LINK_LIBRARIES(libevent ws2_32.lib)

# 设置项目路径
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# 递归获取所有c++文件
SET(HEADER_FILES
	${MODULE_PATH}/stubs/atomicops.h
	${MODULE_PATH}/stubs/atomicops_internals_x86_msvc.h
	${MODULE_PATH}/io/coded_stream.h
	${MODULE_PATH}/io/coded_stream_inl.h
	${MODULE_PATH}/stubs/common.h
)

SET(SOURCE_FILES
	${MODULE_PATH}/stubs/atomicops_internals_x86_msvc.cc
	${MODULE_PATH}/io/coded_stream.cc
	${MODULE_PATH}/stubs/common.cc
	${MODULE_PATH}/extension_set.cc
	${MODULE_PATH}/generated_message_util.cc
	${MODULE_PATH}/message_lite.cc
	${MODULE_PATH}/stubs/once.cc
	${MODULE_PATH}/repeated_field.cc
	${MODULE_PATH}/stubs/stringprintf.cc
	${MODULE_PATH}/wire_format_lite.cc
	${MODULE_PATH}/io/zero_copy_stream.cc
	${MODULE_PATH}/io/zero_copy_stream_impl_lite.cc
)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# 分组文件
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# 预定义宏
ADD_DEFINITIONS(-D_BASE_SERVER_)

# 设置模块名称
SET(MODULE_NAME protobuf-lite)

# 新加库
ADD_LIBRARY(${MODULE_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES} CMakeLists.txt)

# 设置目标属性
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

MESSAGE(STATUS "baseserver configure success!")


