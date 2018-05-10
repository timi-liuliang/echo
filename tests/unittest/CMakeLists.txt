MESSAGE( STATUS "Configuring module: unittest")

# set module name
SET(MODULE_NAME unittest)

# include directories
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty)
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty/googletest/include)

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm *.cc)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

IF(ECHO_UNICODE)
	ADD_DEFINITIONS("-DUNICODE -D_UNICODE")
ENDIF()

# generate module library
ADD_EXECUTABLE(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# link libararies
TARGET_LINK_LIBRARIES(${MODULE_NAME} googletest)

# set folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "tests")

# log
MESSAGE(STATUS "Configure success!")
