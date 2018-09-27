MESSAGE( STATUS "Configuring module: mlpack")

# set module name
SET(MODULE_NAME mlpack)

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty/armadillo/include)
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty/boost_1_68_0/)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm *.cc)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# exclude all main files
EXCLUDE_FILES(ALL_FILES GLOB_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/methods/*_main.cpp)

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# log
MESSAGE(STATUS "Configure success!")
