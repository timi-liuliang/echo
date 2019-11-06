# set module name
SET(MODULE_NAME lzma)

# message
MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# include directories
INCLUDE_DIRECTORIES( ${LORD_SRC_PATH})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.hpp *.c *.cpp *.mm)

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# generate module library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# log
MESSAGE(STATUS "Configure success!")
