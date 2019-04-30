MESSAGE( STATUS "Configuring module: 7zip ...")

# Set Module Name
SET(MODULE_NAME spirv-cross)

# Include Directories
INCLUDE_DIRECTORIES( ${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Recursive get all files
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.c *.cpp *.mm)

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# Message
MESSAGE(STATUS "Configure success!")
