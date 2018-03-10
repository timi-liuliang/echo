/*=============================================================================
FILE: QCompressLib.h

DESCRIPTION: 
	Static library that will be used to compress textures of any format into
	multiple compression formats. 
	
	Files will be written out as a KTX file format.
	
	Library can be used to read in a KTX file and convert to any output format.

	Uses FreeImage to convert into (ensure that freeimage.dll is included in
	the same directory as the final exe).

	AUTHOR:		QUALCOMM
                Copyright (c) 2011 QUALCOMM Incorporated.
                All Rights Reserved.
                QUALCOMM Proprietary/GTDR
=============================================================================*/
#ifndef _QCOMPRESS_LIB_H_
#define _QCOMPRESS_LIB_H_

//
// TODO: Replace TQonvertImage with independant image class that we will hold in the source file
//
#include <TextureConverter.h>

// Use tErrorBuffer to get error code from API functions
#define ERRORBUFFER_MAX 256
typedef char TErrorBuffer[ERRORBUFFER_MAX];

/* GLES3_compressed_texture */
#define GL_COMPRESSED_R11_EAC                                   0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                            0x9271
#define GL_COMPRESSED_RG11_EAC                                  0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                           0x9273
#define GL_COMPRESSED_RGB8_ETC2                                 0x9274
#define GL_COMPRESSED_SRGB8_ETC2                                0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2             0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2            0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                            0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC                     0x9279

/* GL_EXT_texture_compression_dxt1 */
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                         0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                        0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                        0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                        0x83F3

//-----------------------------------------------------------------------------
// HELPER FUNCTIONS
//-----------------------------------------------------------------------------

// In-memory texture containers; mainly used to transition textures from file to mem and from
// format to format in memory
// Main app should only hold pointers to textures and use these functions to alloc and free
// them (note: these functions take care of freeing actual image data)
//
// TODO: Replace TQonvertImage with independant image class that we will hold in the source file
//
TQonvertImage* CreateEmptyTexture();
void           FreeTexture(TQonvertImage* pTexture);

// Functions to transform formats from Q (used by TQonvertImage) to GL (used in KTX files)
unsigned int NumCompressionTypes();
const char  *CompressionName(unsigned int number);

bool FormatQ2GL(const unsigned int QFormat, unsigned int &GLFormat);
bool FormatGL2Q(const unsigned int GLFormat, unsigned int &QFormat);
bool FormatQ2DDS(const unsigned int QFormat, unsigned int &DDSFormat);
bool FormatDDS2Q(const unsigned int DDSFormat, unsigned int &QFormat);
bool FormatName2Q(const char *name, unsigned int &QFormat);
bool IsQFormatValid(const unsigned int GLFormat);

// Return the number of mip maps that would be needed on an image of a given width and height
unsigned int CountNumMipLevels(unsigned int nWidth, unsigned int nHeight);


//-----------------------------------------------------------------------------
// LOAD / SAVE FUNCTIONS
//-----------------------------------------------------------------------------

// Load an image into mem
// Can be any image that FreeImage supports
// Can also be a KTX image that has a known compression set to it (i.e. has been compressed
// and saved using SaveImage)
// If successful, return texture pointer (image always in 32bit RGBA format) and bits-per-pixel 
// of original image (usually 24 or 32)
// NOTE: Width and Height should stay 0 if you want to keep the original size of the image.
// FILE -> MEMORY
bool LoadImage( const char* strFilename, TQonvertImage* pTexture, bool bInvertColor = false, TErrorBuffer err = 0);

// Loads a blank image into memory
bool LoadBlankImage(TQonvertImage* pTexture, unsigned int Width, unsigned int Height, TErrorBuffer err = 0);

// Validate a KTX image. Returns false if image is not recognized or supported.
// If true, nNumMipLevels returns the number of mip maps in the image.
bool ValidateKTXImage (const char *strFilename, unsigned int &nNumMipLevels, TErrorBuffer err = 0);

// Load a KTX file into memory 
// FILE -> MEMORY
bool LoadImageKTX( const char* strFilename, TQonvertImage* pMipTextures, bool IgnoreMipMaps = false, TErrorBuffer err = 0);

// Write out our compressed image
// Image must be in one of the known compressed formats
// MEMORY -> FILE
bool SaveImageKTX(const char* pszOutputFileName, TQonvertImage** pMipTextures, unsigned int nNumMipLevels, TErrorBuffer err = 0);

// Validate a DDS image. Returns false if image is not recognized or supported.
// If true, nNumMipLevels returns the number of mip maps in the image.
bool ValidateDDSImage (const char *strFilename, unsigned int &nNumMipLevels, TErrorBuffer err = 0);

// Load a DDS file into memory 
// FILE -> MEMORY
bool LoadImageDDS(const char* strFilename, TQonvertImage* pMipTextures, bool IgnoreMipMaps = false, TErrorBuffer err = 0);

// Write out our compressed image into DDS format
// Image must be in one of the known compressed formats
// MEMORY -> FILE
bool SaveImageDDS(const char* pszOutputFileName, TQonvertImage** pMipTextures, unsigned int nNumMipLevels, TErrorBuffer err = 0);

// Write out specifically to a Targa image
// MEMORY -> FILE
bool SaveImageTGA( const char* pszOutputFileName, TQonvertImage* pTexture, TErrorBuffer err = 0);

// Write out raw binary data within a TQonvertImage
// MEMORY -> FILE
bool SaveImageRAW(const char* pszOutputFileName, TQonvertImage* pTexture, TErrorBuffer err = 0);


//-----------------------------------------------------------------------------
// COMPRESSION / DECOMPRESSION FUNCTIONS
//-----------------------------------------------------------------------------

// MEMORY -> MEMORY
// NOTE: To resize an image, use pTextureIn->nFormat as the value for 
// Compress function argument 'QFormat' below
bool Compress (TQonvertImage* pTextureIn, 
			   TQonvertImage* pTextureOut, 
			   unsigned int QFormat,
			   unsigned int Width, unsigned int Height, 
			   TErrorBuffer = 0);

// MEMORY -> MEMORY
bool Decompress (TQonvertImage* pTextureIn, 
				 TQonvertImage* pTextureOut, 
				 unsigned int Width, unsigned int Height,
				 TErrorBuffer err = 0);

// Make sure to use CountNumMipLevels to figure out how many mip levels is needed
// and to create an array of 'TQonvertImage's for mip textures using the 
// function CreateEmptyTexture
// MEMORY -> MEMORY
bool MipMapAndCompress(TQonvertImage* pTexture, 
					   TQonvertImage** pMipTextures, 
					   unsigned int QFormat,
					   unsigned int nWidth, unsigned int nHeight, 
					   unsigned int nNumMipLevels,
					   TErrorBuffer err = 0);

#endif // QCOMPRESS_LIB_H