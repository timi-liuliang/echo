# ---- #
# META #
# ---- #

cmake_minimum_required(VERSION 3.6)


# ------------ #
# PUBLIC LISTS #
# ------------ #

get_directory_property(_CSM_CORE_PARENT_SCOPE PARENT_DIRECTORY)
if(_CSM_CORE_PARENT_SCOPE)
  # Will contain path to include directory if building for native.
  set(CSM_CORE_INCLUDE_DIR "" PARENT_SCOPE)

  # Will contain link libraries if building for native.
  set(CSM_CORE_LIBS "" PARENT_SCOPE)

  # Will contain dependencies if building for native.
  set(CSM_CORE_DEPS "" PARENT_SCOPE)
endif ()


# ----------------------- #
# OPTIONS INTERNALIZATION #
# ----------------------- #

# Somewhat detect macOS...
if (APPLE AND NOT IOS)
  set(MACOS ON)


# Somewhat detect Linux...
elseif (UNIX AND NOT IOS AND NOT RPI)
  set(LINUX ON)
endif ()


# --------- #
# LIBRARIES #
# --------- #

# Find directory containing library.
if (ANDROID AND (ANDROID_ABI STREQUAL "armeabi-v7a"))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/android/armeabi-v7a)
elseif (ANDROID AND (ANDROID_ABI STREQUAL "arm64-v8a"))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/android/arm64-v8a)
elseif (ANDROID AND (ANDROID_ABI STREQUAL "x86"))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/android/x86)
elseif (EMSCRIPTEN)
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/experimental/emscripten)
elseif (IOS)
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/ios)
elseif (LINUX AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/linux/x86_64)
elseif (MACOS)
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/macos)
elseif (RPI)
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/experimental/rpi)
elseif (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/windows/x86)
elseif (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 8))
  set(_CSM_CORE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/windows/x86_64)
endif ()


# Make sure library exists for target platform.
if (NOT _CSM_CORE_LIB_DIR)
  message(FATAL_ERROR "[Live2D Cubism Core] Library not found for target platform.")
endif ()


# Initialize public lists.
if(_CSM_CORE_PARENT_SCOPE)
  set(CSM_CORE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include PARENT_SCOPE)


  if (EMSCRIPTEN)
    set(CSM_CORE_LIBS ${_CSM_CORE_LIB_DIR}/Live2DCubismCore.bc PARENT_SCOPE)
  else ()
    set(CSM_CORE_LIBS ${_CSM_CORE_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}Live2DCubismCore${CMAKE_STATIC_LIBRARY_SUFFIX} PARENT_SCOPE)
  endif ()


  if (NOT WIN32)
    list(APPEND CSM_CORE_LIBS m)
  endif ()


  set(CSM_CORE_DEPS OFF PARENT_SCOPE)
endif ()
