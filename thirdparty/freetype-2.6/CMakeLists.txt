MESSAGE(STATUS "configuring module: freetype...")

# 附加包含目录
SET(INCLUDE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}
	${ECHO_ROOT_PATH}/ThirdParty/FreeImage/LibPNG
)

INCLUDE_DIRECTORIES( ${INCLUDE_FILES})

# 链接库
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
LINK_LIBRARIES()

# 设置项目路径
SET(MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


# 源文件路径
SET(FREETYPE_HEADER_PATH ${MODULE_PATH}/include)
SET(FREETYPE_SOURCE_PATH ${MODULE_PATH}/src)

# 预定义宏
ADD_DEFINITIONS(-DFT_DEBUG_LEVEL_ERROR -DFT_DEBUG_LEVEL_TRACE -DFT2_BUILD_LIBRARY)
IF(ECHO_BUILD_PLATFORM_ANDROID)
	ADD_DEFINITIONS(-DFT_CONFIG_OPTION_SYSTEM_ZLIB)
ENDIF()

# Windows平台FreeType文件
SET(FREETYPE_HEADER_FILES
	${FREETYPE_HEADER_PATH}/ft2build.h
	${FREETYPE_HEADER_PATH}/freetype/config/ftconfig.h
	${FREETYPE_HEADER_PATH}/freetype/config/ftheader.h
	${FREETYPE_HEADER_PATH}/freetype/config/ftmodule.h
	${FREETYPE_HEADER_PATH}/freetype/config/ftoption.h
	${FREETYPE_HEADER_PATH}/freetype/config/ftstdlib.h
  )
SET(FREETYPE_SOURCE_FILES
	${FREETYPE_SOURCE_PATH}/base/ftbbox.c
	${FREETYPE_SOURCE_PATH}/base/ftfntfmt.c
	${FREETYPE_SOURCE_PATH}/base/ftgxval.c
	${FREETYPE_SOURCE_PATH}/base/ftlcdfil.c
	${FREETYPE_SOURCE_PATH}/base/ftmm.c
	${FREETYPE_SOURCE_PATH}/base/ftotval.c
	${FREETYPE_SOURCE_PATH}/base/ftpatent.c
	${FREETYPE_SOURCE_PATH}/base/ftpfr.c
	${FREETYPE_SOURCE_PATH}/base/ftsynth.c
	${FREETYPE_SOURCE_PATH}/base/fttype1.c
	${FREETYPE_SOURCE_PATH}/base/ftwinfnt.c
	${FREETYPE_SOURCE_PATH}/pcf/pcf.c
	${FREETYPE_SOURCE_PATH}/pfr/pfr.c
	${FREETYPE_SOURCE_PATH}/psaux/psaux.c
	${FREETYPE_SOURCE_PATH}/pshinter/pshinter.c
	${FREETYPE_SOURCE_PATH}/psnames/psmodule.c
	${FREETYPE_SOURCE_PATH}/raster/raster.c
	${FREETYPE_SOURCE_PATH}/sfnt/sfnt.c
	${FREETYPE_SOURCE_PATH}/truetype/truetype.c
	${FREETYPE_SOURCE_PATH}/type1/type1.c
	${FREETYPE_SOURCE_PATH}/cid/type1cid.c
	${FREETYPE_SOURCE_PATH}/type42/type42.c
	${FREETYPE_SOURCE_PATH}/winfonts/winfnt.c
	${FREETYPE_SOURCE_PATH}/autofit/autofit.c
	${FREETYPE_SOURCE_PATH}/bdf/bdf.c
	${FREETYPE_SOURCE_PATH}/cff/cff.c
	${FREETYPE_SOURCE_PATH}/base/ftbase.c
	${FREETYPE_SOURCE_PATH}/base/ftbitmap.c
	${FREETYPE_SOURCE_PATH}/cache/ftcache.c
	${FREETYPE_SOURCE_PATH}/base/ftdebug.c
	${FREETYPE_SOURCE_PATH}/base/ftfstype.c
	${FREETYPE_SOURCE_PATH}/base/ftgasp.c
	${FREETYPE_SOURCE_PATH}/base/ftglyph.c
	${FREETYPE_SOURCE_PATH}/base/ftglyph.c
	${FREETYPE_SOURCE_PATH}/gzip/ftgzip.c
	${FREETYPE_SOURCE_PATH}/base/ftinit.c
	${FREETYPE_SOURCE_PATH}/lzw/ftlzw.c
	${FREETYPE_SOURCE_PATH}/base/ftstroke.c
	${FREETYPE_SOURCE_PATH}/base/ftsystem.c
	${FREETYPE_SOURCE_PATH}/smooth/smooth.c
)

SET(ALL_FILES ${FREETYPE_HEADER_FILES} ${FREETYPE_SOURCE_FILES})

# 分组文件
GROUP_FILES(ALL_FILES ${CMAKE_CURRENT_SOURCE_DIR})

# 设置模块名称
SET(MODULE_NAME freetype)

# 新加库
ADD_LIBRARY(${MODULE_NAME} STATIC ${ALL_FILES} CMakeLists.txt)

# 设置目标属性
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

MESSAGE(STATUS "baseserver configure success!")


