MESSAGE( STATUS "Configuring ThirdParty: freeimage...")

# Set Module Name
SET(MODULE_NAME freeimage)

# Include directories
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES( ${ECHO_ROOT_PATH}/thirdparty/zlib/)

# link directories
LINK_DIRECTORIES(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# predefine macros
ADD_DEFINITIONS( -DLIBRAW_NODLL -DDISABLE_PERF_MEASUREMENT -DOPJ_STATIC -DNOMINMAX)

# Set source file paths
SET(FREEIMAG_MODULE_PATH        ${ECHO_ROOT_PATH}/thirdparty/FreeImage)
SET(FREEIMAG_MODULE_HEADER_PATH ${ECHO_ROOT_PATH}/thirdparty/FreeImage)
SET(FREEIMAG_MODULE_SOURCE_PATH ${ECHO_ROOT_PATH}/thirdparty/FreeImage)

# 递归查找文件
FILE(GLOB_RECURSE FREEIMAGE_HEADER_FILES ${FREEIMAG_MODULE_HEADER_PATH}/FreeImage/*.h ${FREEIMAG_MODULE_HEADER_PATH}/FreeImage/*.inl)
SET(FREEIMAGE_SOURCE_FILES
	${FREEIMAG_MODULE_PATH}/FreeImage/BitmapAccess.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/Conversion16_565.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/ColorLookup.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/FreeImage.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/FreeImageC.c ${FREEIMAG_MODULE_PATH}/FreeImage/FreeImageIO.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/GetType.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/MemoryIO.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PixelAccess.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/J2KHelper.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/MNGHelper.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/Plugin.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginBMP.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginCUT.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginDDS.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginEXR.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginG3.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginGIF.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginHDR.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginICO.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginIFF.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginJ2K.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginJNG.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginJP2.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginJPEG.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginJXR.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginKOALA.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginMNG.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginPCD.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginPCX.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginPFM.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginPICT.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginPNG.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginPNM.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginPSD.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginRAS.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginRAW.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginSGI.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginTARGA.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginTIFF.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginWBMP.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginWebP.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PluginXBM.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/PluginXPM.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/PSDParser.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/TIFFLogLuv.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/Conversion.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/Conversion16_555.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/Conversion24.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/Conversion32.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/Conversion4.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/Conversion8.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/ConversionFloat.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/ConversionRGB16.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/ConversionRGBF.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/ConversionType.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/ConversionUINT16.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/Halftoning.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/tmoColorConvert.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/tmoDrago03.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/tmoFattal02.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/tmoReinhard05.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/ToneMapping.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/NNQuantizer.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/WuQuantizer.cpp
	${FREEIMAG_MODULE_PATH}/DeprecationManager/Deprecated.cpp ${FREEIMAG_MODULE_PATH}/DeprecationManager/DeprecationMgr.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/CacheFile.cpp ${FREEIMAG_MODULE_PATH}/FreeImage/MultiPage.cpp
	${FREEIMAG_MODULE_PATH}/FreeImage/ZLibInterface.cpp ${FREEIMAG_MODULE_PATH}/Metadata/Exif.cpp
	${FREEIMAG_MODULE_PATH}/Metadata/FIRational.cpp ${FREEIMAG_MODULE_PATH}/Metadata/FreeImageTag.cpp ${FREEIMAG_MODULE_PATH}/Metadata/IPTC.cpp
	${FREEIMAG_MODULE_PATH}/Metadata/TagConversion.cpp ${FREEIMAG_MODULE_PATH}/Metadata/TagLib.cpp
	${FREEIMAG_MODULE_PATH}/Metadata/XTIFF.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Background.cpp
	${FREEIMAG_MODULE_PATH}/FreeImageToolkit/BSplineRotate.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Channels.cpp
	${FREEIMAG_MODULE_PATH}/FreeImageToolkit/ClassicRotate.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Colors.cpp
	${FREEIMAG_MODULE_PATH}/FreeImageToolkit/CopyPaste.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Display.cpp
	${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Flip.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/JPEGTransform.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/MultigridPoissonSolver.cpp
	${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Rescale.cpp ${FREEIMAG_MODULE_PATH}/FreeImageToolkit/Resize.cpp
	${FREEIMAG_MODULE_PATH}/LibJPEG/jaricom.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcapimin.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcapistd.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jcarith.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jccoefct.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jccolor.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jcdctmgr.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jchuff.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcinit.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jcmainct.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcmarker.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcmaster.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jcomapi.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcparam.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jcprepct.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jcsample.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jctrans.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdapimin.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jdapistd.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdarith.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdatadst.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jdatasrc.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdcoefct.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdcolor.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jddctmgr.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdhuff.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdinput.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jdmainct.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdmarker.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdmaster.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jdmerge.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdpostct.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jdsample.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jdtrans.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jerror.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jfdctflt.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jfdctfst.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jfdctint.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jidctflt.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jidctfst.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jidctint.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jmemmgr.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jmemnobs.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/jquant1.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jquant2.c ${FREEIMAG_MODULE_PATH}/LibJPEG/jutils.c
	${FREEIMAG_MODULE_PATH}/LibJPEG/transupp.c ${FREEIMAG_MODULE_PATH}/LibPNG/png.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngerror.c
	${FREEIMAG_MODULE_PATH}/LibPNG/pngget.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngmem.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngpread.c
	${FREEIMAG_MODULE_PATH}/LibPNG/pngread.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngrio.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngrtran.c
	${FREEIMAG_MODULE_PATH}/LibPNG/pngrutil.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngset.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngtrans.c
	${FREEIMAG_MODULE_PATH}/LibPNG/pngwio.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngwrite.c ${FREEIMAG_MODULE_PATH}/LibPNG/pngwtran.c
	${FREEIMAG_MODULE_PATH}/LibPNG/pngwutil.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_aux.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_close.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_codec.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_color.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_compress.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_dir.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_dirinfo.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_dirread.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_dirwrite.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_dumpmode.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_error.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_extension.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_fax3.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_fax3sm.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_flush.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_getimage.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_jpeg.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_luv.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_lzma.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_lzw.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_next.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_ojpeg.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_open.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_packbits.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_pixarlog.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_predict.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_print.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_read.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_strip.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_swab.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_thunder.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_tile.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_version.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_warning.c ${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_write.c
	${FREEIMAG_MODULE_PATH}/LibTIFF4/tif_zip.c  ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/bio.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/cio.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/dwt.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/event.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/function_list.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/image.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/invert.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/j2k.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/jp2.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/mct.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/mqc.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/openjpeg.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/opj_clock.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/pi.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/raw.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/t1.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/t2.c ${FREEIMAG_MODULE_PATH}/LibOpenJPEG/tcd.c
	${FREEIMAG_MODULE_PATH}/LibOpenJPEG/tgt.c
	)

IF(FREEIMAGE_SUPPORT_OPENEXR)
	ADD_DEFINITIONS(-DFREEIMAGE_SUPPORT_OPENEXR)

	SET(FREEIMAGE_SOURCE_FILES ${FREEIMAGE_SOURCE_FILES}
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfB44Compressor.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfBoxAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfChannelList.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfChannelListAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfChromaticities.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfChromaticitiesAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfCompressionAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfCompressor.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfConvert.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfCRgbaFile.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfDoubleAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfEnvmap.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfEnvmapAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfFloatAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfFrameBuffer.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfFramesPerSecond.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfHeader.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfHuf.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfInputFile.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfIntAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfIO.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfKeyCode.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfKeyCodeAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfLineOrderAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfLut.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfMatrixAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfMisc.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfOpaqueAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfOutputFile.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfPizCompressor.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfPreviewImage.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfPreviewImageAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfPxr24Compressor.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfRational.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfRationalAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfRgbaFile.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfRgbaYca.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfRleCompressor.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfScanLineInputFile.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfStandardAttributes.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfStdIO.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfStringAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfStringVectorAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTestFile.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfThreading.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTileDescriptionAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTiledInputFile.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTiledMisc.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTiledOutputFile.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTiledRgbaFile.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTileOffsets.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTimeCode.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfTimeCodeAttribute.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfVecAttribute.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfVersion.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfWav.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmImf/ImfZipCompressor.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathBox.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathColorAlgo.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathFun.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathMatrixAlgo.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathRandom.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathShear.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Imath/ImathVec.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Iex/IexBaseExc.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/Iex/IexThrowErrnoExc.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/Half/half.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmThread/IlmThread.cpp
	${FREEIMAG_MODULE_PATH}/OpenEXR/IlmThread/IlmThreadMutex.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmThread/IlmThreadPool.cpp ${FREEIMAG_MODULE_PATH}/OpenEXR/IlmThread/IlmThreadSemaphore.cpp
	)
ELSE()
	SET(FREEIMAGE_SOURCE_FILES ${FREEIMAGE_SOURCE_FILES}
		${FREEIMAG_MODULE_PATH}/OpenEXR/Half/half.cpp
	)
ENDIF(FREEIMAGE_SUPPORT_OPENEXR)

IF(FREEIMAGE_SUPPORT_LIBRAW)
	ADD_DEFINITIONS(-DFREEIMAGE_SUPPORT_LIBRAW)

	SET(FREEIMAGE_SOURCE_FILES ${FREEIMAGE_SOURCE_FILES}
	${FREEIMAG_MODULE_PATH}/LibRawLite/internal/dcraw_common.cpp ${FREEIMAG_MODULE_PATH}/LibRawLite/internal/dcraw_fileio.cpp ${FREEIMAG_MODULE_PATH}/LibRawLite/internal/demosaic_packs.cpp
	${FREEIMAG_MODULE_PATH}/LibRawLite/src/libraw_c_api.cpp ${FREEIMAG_MODULE_PATH}/LibRawLite/src/libraw_cxx.cpp ${FREEIMAG_MODULE_PATH}/LibRawLite/src/libraw_datastream.cpp
	)
ENDIF(FREEIMAGE_SUPPORT_LIBRAW)

IF(FREEIMAGE_SUPPORT_WEBP)
	ADD_DEFINITIONS(-DFREEIMAGE_SUPPORT_WEBP)
	SET(FREEIMAGE_SOURCE_FILES ${FREEIMAGE_SOURCE_FILES}
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_alpha.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_buffer.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_frame.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_idec.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_io.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_layer.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_quant.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_tree.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_vp8.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_vp8l.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dec/dec_webp.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/cpu.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/dec.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/dec_clip_tables.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/dec_neon.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/dec_sse2.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/enc.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/enc_neon.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/enc_sse2.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/lossless.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/upsampling.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/upsampling_neon.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/upsampling_sse2.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/dsp/yuv.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_alpha.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_analysis.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_backward_references.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_config.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_cost.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_filter.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_frame.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_histogram.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_iterator.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_layer.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_picture.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_quant.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_syntax.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_tree.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_vp8l.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/enc_webpenc.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/enc/token.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/alpha_processing.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/bit_reader.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/bit_writer.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/color_cache.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/filters.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/huffman.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/huffman_encode.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/quant_levels.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/quant_levels_dec.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/random.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/rescaler.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/thread.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/utils/utils.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/mux/muxedit.c
	${FREEIMAG_MODULE_PATH}/LibWebP/src/mux/muxinternal.c ${FREEIMAG_MODULE_PATH}/LibWebP/src/mux/muxread.c
	)
ENDIF(FREEIMAGE_SUPPORT_WEBP)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_LIST_DIR}
	${FREEIMAG_MODULE_PATH}
	${FREEIMAG_MODULE_PATH}/OpenEXR
	${FREEIMAG_MODULE_PATH}/LibRawLite
	)

SET(ALL_FILES ${FREEIMAGE_HEADER_FILES} ${FREEIMAGE_SOURCE_FILES})

# Group
GROUP_FILES(ALL_FILES ${FREEIMAG_MODULE_PATH})

# Generate library
ADD_LIBRARY(${MODULE_NAME} ${ALL_FILES} CMakeLists.txt)

# Set directories
SET_TARGET_PROPERTIES(${MODULE_NAME} PROPERTIES FOLDER "thirdparty")

# Message
MESSAGE(STATUS "Configure success!")
