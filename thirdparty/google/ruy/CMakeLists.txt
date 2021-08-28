# set module name
SET(MODULE_NAME ruy)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Set Files
IF(ECHO_PLATFORM_WINDOWS)
	SET(ALL_FILES
		${CMAKE_CURRENT_SOURCE_DIR}/ruy/allocator.cc
	)
ENDIF()

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Definition
IF(WIN32)
    ADD_DEFINITIONS(-DNOMINMAX -DWIN64 -DWIN32_LEAN_AND_MEAN -DNOGDI)
    ADD_DEFINITIONS(/bigobj /nologo /EHsc /GF /MP /Gm- /wd4800 /wd4805 /wd4244)
    IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # if we compile for windows with clang, allow inline asm
        ADD_DEFINITIONS(-DGEMMLOWP_ALLOW_INLINE_ASM)
    ENDIF()
ENDIF()

# CXX Statndard
SET(CMAKE_CXX_STANDARD 11)

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# log
MESSAGE(STATUS "Configure success!")
