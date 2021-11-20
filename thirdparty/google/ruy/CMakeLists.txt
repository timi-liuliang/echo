# set module name
SET(MODULE_NAME ruy)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# Some components require C++14.
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Set Files
FILE(GLOB ALL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ruy/*.*")
LIST(FILTER ALL_FILES INCLUDE REGEX ".*\\.(c|cc|h)$")
LIST(FILTER ALL_FILES EXCLUDE REGEX ".*(_test)\\.(c|cc|h)$")
LIST(FILTER ALL_FILES EXCLUDE REGEX ".*/(benchmark|example|test_.*)\.cc$")
LIST(FILTER ALL_FILES EXCLUDE REGEX ".*/gtest_wrapper\\.h$")

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Definition
IF(WIN32)
    ADD_DEFINITIONS(-DNOMINMAX -DWIN64 -DWIN32_LEAN_AND_MEAN -DNOGDI -D_WINDOWS)
    ADD_DEFINITIONS(/bigobj /nologo /EHsc /GF /MP /Gm- /wd4800 /wd4805 /wd4244)
    IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # if we compile for windows with clang, allow inline asm
        ADD_DEFINITIONS(-DGEMMLOWP_ALLOW_INLINE_ASM)
    ENDIF()
ENDIF()

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# log
MESSAGE(STATUS "Configure success!")
