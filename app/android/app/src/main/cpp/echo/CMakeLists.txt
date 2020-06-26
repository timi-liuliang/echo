# set module name
SET(MODULE_NAME echojni)

# include directories
INCLUDE_DIRECTORIES(${ECHO_ROOT_PATH})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# link
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# recursive get all module files
FILE( GLOB_RECURSE HEADER_FILES *.h *.inl *.hpp)
FILE( GLOB_RECURSE SOURCE_FILES *.cpp *.mm)

SET(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})

# group files by folder
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add a shared library
ADD_LIBRARY(${MODULE_NAME} SHARED ${ALL_FILES} CMakeLists.txt)

# Link engine libraries
TARGET_LINK_LIBRARIES(${MODULE_NAME} engine)
TARGET_LINK_LIBRARIES(${MODULE_NAME} lua pugixml freeimage box2d freetype physx spine zlib)
TARGET_LINK_LIBRARIES(${MODULE_NAME} android log EGL GLESv2)
TARGET_LINK_LIBRARIES(${MODULE_NAME} glslang spirv-cross)