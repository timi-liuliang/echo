# set module name
SET(MODULE_NAME farmhash)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/google)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm *.cc *.c)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# definitions
ADD_DEFINITIONS("-DFARMHASH_NO_BUILTIN_EXPECT")

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty/google")

# log
MESSAGE(STATUS "Configure success!")
