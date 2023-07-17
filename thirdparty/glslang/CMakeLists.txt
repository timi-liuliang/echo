# Set Module Name
SET(MODULE_NAME glslang)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# Include Directories
INCLUDE_DIRECTORIES( ${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# set module path
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Recursive get all files
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.c *.cpp *.mm)

# remove files by platform
IF(ECHO_PLATFORM_WINDOWS)
    EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${MODULE_PATH}/glslang/OSDependent/Unix/*.*)
ELSE()
    EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${MODULE_PATH}/glslang/OSDependent/Windows/*.*)
ENDIF()

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/khronosgroup")

# Message
MESSAGE(STATUS "Configure success!")
