# Set Module Name
SET(MODULE_NAME libde265)

MESSAGE( STATUS "Configuring module: ${MODULE_NAME} ...")

# include directories
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/extra)

# link directory
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all files
FILE( GLOB_RECURSE ALL_FILES *.h *.inl *.c *.cpp *.mm *.cc)

# group files
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES})

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# end message
MESSAGE(STATUS "Configure ${MODULE_NAME} success!")
