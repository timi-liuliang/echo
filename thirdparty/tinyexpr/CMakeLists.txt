# Set Module Name
SET(MODULE_NAME tinyexpr)

# Begin configure module
MESSAGE( STATUS "Configuring module: ${MODULE_NAME}...")

# Include Directories
INCLUDE_DIRECTORIES( ${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Recursive get all files
FILE( GLOB_RECURSE ALL_FILES *.h *.hpp *.inl *.c *.cpp *.mm)

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# MSVC fix (error C2099 "initializer is not a constant" in Release builds):
#   With the DLL runtime (/MD) the CRT declares fabs()/ceil()/floor() as __declspec(dllimport),
#   and /O2 (which implies /Oi) turns them into compiler intrinsics. The address of such an
#   intrinsic is not a constant expression, so the static function table in tinyexpr.c
#   (the "abs"/"ceil"/"floor" entries) fails to compile. Disabling intrinsic generation for
#   this target only makes them ordinary function pointers again. The third-party source stays
#   untouched so it keeps syncing cleanly with upstream.
IF(MSVC)
    TARGET_COMPILE_OPTIONS(${MODULE_NAME} PRIVATE /Oi-)
ENDIF()

# Message
MESSAGE(STATUS "Configure success!")
