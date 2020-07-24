# Set Module Name
SET(MODULE_NAME openal-soft)

# Define Module Path
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Begin configure module
MESSAGE( STATUS "Configuring module: ${MODULE_NAME}...")

# Include Directories
INCLUDE_DIRECTORIES( ${ECHO_SRC_PATH})
INCLUDE_DIRECTORIES( ${MODULE_PATH})
INCLUDE_DIRECTORIES( ${MODULE_PATH}/include)
INCLUDE_DIRECTORIES( ${MODULE_PATH}/alc)

# Link Directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Add Definitions
ADD_DEFINITIONS(-DNOMINMAX)
ADD_DEFINITIONS(-DAL_ALEXT_PROTOTYPES)

# config
IF(ECHO_PLATFORM_WIN32)
    SET(ALSOFT_EMBED_HRTF_DATA TRUE)
    SET(HAVE_WASAPI TRUE)
    SET(HAVE_WAVE TRUE)
    SET(HAVE_MALLOC_H TRUE)
    SET(HAVE_INTRIN_H TRUE)
    SET(HAVE_GUIDDEF_H TRUE)
    SET(HAVE_CPUID_INTRINSIC TRUE)
    SET(HAVE_BITSCANFORWARD_INTRINSIC, TRUE)
    SET(HAVE_BITSCANFORWARD64_INTRINSIC TRUE)
    SET(HAVE_SSE_INTRINSICS TRUE)
    SET(SIZEOF_LONG 4)
    SET(HAVE_WINMM TRUE)
    SET(HAVE__ALIGNED_MALLOC TRUE)
    SET(HAVE_STAT TRUE)
ELSEIF(ECHO_PLATFORM_ANDROID)
    SET(HAVE_OPENSL TRUE)
ENDIF()

# Configure FILE
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
)

# Common Files
SET(COMMON_FILES
    ${MODULE_PATH}/common/albyte.h
    ${MODULE_PATH}/common/alcomplex.cpp
    ${MODULE_PATH}/common/alcomplex.h
    ${MODULE_PATH}/common/alexcpt.cpp
    ${MODULE_PATH}/common/alexcpt.h
    ${MODULE_PATH}/common/alfstream.cpp
    ${MODULE_PATH}/common/alfstream.h
    ${MODULE_PATH}/common/almalloc.cpp
    ${MODULE_PATH}/common/almalloc.h
    ${MODULE_PATH}/common/alnumeric.h
    ${MODULE_PATH}/common/aloptional.h
    ${MODULE_PATH}/common/alspan.h
    ${MODULE_PATH}/common/alstring.cpp
    ${MODULE_PATH}/common/alstring.h
    ${MODULE_PATH}/common/atomic.h
    ${MODULE_PATH}/common/dynload.cpp
    ${MODULE_PATH}/common/dynload.h
    ${MODULE_PATH}/common/endiantest.h
    ${MODULE_PATH}/common/intrusive_ptr.h
    ${MODULE_PATH}/common/math_defs.h
    ${MODULE_PATH}/common/opthelpers.h
    ${MODULE_PATH}/common/polyphase_resampler.cpp
    ${MODULE_PATH}/common/polyphase_resampler.h
    ${MODULE_PATH}/common/pragmadefs.h
    ${MODULE_PATH}/common/strutils.cpp
    ${MODULE_PATH}/common/strutils.h
    ${MODULE_PATH}/common/threads.cpp
    ${MODULE_PATH}/common/threads.h
    ${MODULE_PATH}/common/vecmat.h
    ${MODULE_PATH}/common/vector.h
)

# OpenAl Files
SET(OPENAL_FILES
    ${MODULE_PATH}/al/auxeffectslot.cpp
    ${MODULE_PATH}/al/auxeffectslot.h
    ${MODULE_PATH}/al/buffer.cpp
    ${MODULE_PATH}/al/buffer.h
    ${MODULE_PATH}/al/effect.cpp
    ${MODULE_PATH}/al/effect.h
    ${MODULE_PATH}/al/error.cpp
    ${MODULE_PATH}/al/event.cpp
    ${MODULE_PATH}/al/event.h
    ${MODULE_PATH}/al/extension.cpp
    ${MODULE_PATH}/al/filter.cpp
    ${MODULE_PATH}/al/filter.h
    ${MODULE_PATH}/al/listener.cpp
    ${MODULE_PATH}/al/listener.h
    ${MODULE_PATH}/al/source.cpp
    ${MODULE_PATH}/al/source.h
    ${MODULE_PATH}/al/state.cpp
)

# ALC Files
SET(ALC_FILES
    ${MODULE_PATH}/alc/alc.cpp
    ${MODULE_PATH}/alc/alcmain.h
    ${MODULE_PATH}/alc/alu.cpp
    ${MODULE_PATH}/alc/alu.h
    ${MODULE_PATH}/alc/alconfig.cpp
    ${MODULE_PATH}/alc/alconfig.h
    ${MODULE_PATH}/alc/alcontext.h
    ${MODULE_PATH}/alc/ambdec.cpp
    ${MODULE_PATH}/alc/ambdec.h
    ${MODULE_PATH}/alc/ambidefs.h
    ${MODULE_PATH}/alc/bformatdec.cpp
    ${MODULE_PATH}/alc/bformatdec.h
    ${MODULE_PATH}/alc/bs2b.cpp
    ${MODULE_PATH}/alc/bs2b.h
    ${MODULE_PATH}/alc/compat.h
    ${MODULE_PATH}/alc/converter.cpp
    ${MODULE_PATH}/alc/converter.h
    ${MODULE_PATH}/alc/cpu_caps.h
    ${MODULE_PATH}/alc/devformat.h
    ${MODULE_PATH}/alc/effects/base.h
    ${MODULE_PATH}/alc/effects/autowah.cpp
    ${MODULE_PATH}/alc/effects/chorus.cpp
    ${MODULE_PATH}/alc/effects/compressor.cpp
    ${MODULE_PATH}/alc/effects/dedicated.cpp
    ${MODULE_PATH}/alc/effects/distortion.cpp
    ${MODULE_PATH}/alc/effects/echo.cpp
    ${MODULE_PATH}/alc/effects/equalizer.cpp
    ${MODULE_PATH}/alc/effects/fshifter.cpp
    ${MODULE_PATH}/alc/effects/modulator.cpp
    ${MODULE_PATH}/alc/effects/null.cpp
    ${MODULE_PATH}/alc/effects/pshifter.cpp
    ${MODULE_PATH}/alc/effects/reverb.cpp
    ${MODULE_PATH}/alc/effects/vmorpher.cpp
    ${MODULE_PATH}/alc/filters/biquad.h
    ${MODULE_PATH}/alc/filters/biquad.cpp
    ${MODULE_PATH}/alc/filters/nfc.cpp
    ${MODULE_PATH}/alc/filters/nfc.h
    ${MODULE_PATH}/alc/filters/splitter.cpp
    ${MODULE_PATH}/alc/filters/splitter.h
    ${MODULE_PATH}/alc/fpu_modes.h
    ${MODULE_PATH}/alc/helpers.cpp
    ${MODULE_PATH}/alc/hrtf.cpp
    ${MODULE_PATH}/alc/hrtf.h
    ${MODULE_PATH}/alc/inprogext.h
    ${MODULE_PATH}/alc/logging.h
    ${MODULE_PATH}/alc/mastering.cpp
    ${MODULE_PATH}/alc/mastering.h
    ${MODULE_PATH}/alc/panning.cpp
    ${MODULE_PATH}/alc/ringbuffer.cpp
    ${MODULE_PATH}/alc/ringbuffer.h
    ${MODULE_PATH}/alc/uhjfilter.cpp
    ${MODULE_PATH}/alc/uhjfilter.h
    ${MODULE_PATH}/alc/uiddefs.cpp
    ${MODULE_PATH}/alc/voice.cpp
    ${MODULE_PATH}/alc/voice.h
    ${MODULE_PATH}/alc/mixer/defs.h
    ${MODULE_PATH}/alc/mixer/hrtfbase.h
    ${MODULE_PATH}/alc/mixer/mixer_c.cpp
)

SET(BACKEND
    ${MODULE_PATH}/alc/backends/base.cpp
    ${MODULE_PATH}/alc/backends/base.h
    # Default backends, always available
    ${MODULE_PATH}/alc/backends/loopback.cpp
    ${MODULE_PATH}/alc/backends/loopback.h
    ${MODULE_PATH}/alc/backends/null.cpp
    ${MODULE_PATH}/alc/backends/null.h
)

SET(WASAPI_BACKEND
    ${MODULE_PATH}/alc/backends/wasapi.cpp
    ${MODULE_PATH}/alc/backends/wasapi.h
)

SET(WINMM_BACKEND
    ${MODULE_PATH}/alc/backends/winmm.cpp
    ${MODULE_PATH}/alc/backends/winmm.h
)

SET(WAVE_BACKEND
    ${MODULE_PATH}/alc/backends/wave.cpp
    ${MODULE_PATH}/alc/backends/wave.h
)

# Recursive get all files
SET(ALL_FILES 
    ${COMMON_FILES} ${OPENAL_FILES} ${ALC_FILES} 
    ${BACKEND} ${WASAPI_BACKEND} ${WINMM_BACKEND} ${WAVE_BACKEND}
)

# Group
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# Add library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set Folder
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# Message
MESSAGE(STATUS "Configure success!")
