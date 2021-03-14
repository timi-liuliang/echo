# Set Module Name
SET(MODULE_NAME tensorflow-lite)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# Include Directories
INCLUDE_DIRECTORIES( ${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Recursive get all files
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.c *.cc *.cpp *.mm)

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/tensorflow")

# Message
MESSAGE(STATUS "Configure success!")
