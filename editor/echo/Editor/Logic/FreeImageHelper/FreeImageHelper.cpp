#include "FreeImageHelper.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
	static FreeImageHelper g_freeImageHelper;

	FreeImageHelper::FreeImageHelper()
	{
		FreeImage_Initialise();
	}

	FreeImageHelper::~FreeImageHelper()
	{
		FreeImage_DeInitialise();
	}

    FreeImageHelper* FreeImageHelper::instance()
    {
        static FreeImageHelper* inst = EchoNew(FreeImageHelper);
        return inst;
    }

	int FreeImageHelper::mappingFlagsByFormat( FREE_IMAGE_FORMAT fmt)
	{
		int fiFlags = 0;
		if( fmt==FIF_PNG)
			fiFlags = PNG_DEFAULT;
		else if( fmt==FIF_JPEG)
			fiFlags = JPEG_DEFAULT;
		else if( fmt==FIF_TARGA)
			fiFlags = TARGA_DEFAULT;
		else if( fmt==FIF_BMP)
			fiFlags = BMP_DEFAULT;

		return fiFlags;
	}

	bool FreeImageHelper::getImageInfo( const char* filePath, ImageInfo& imageInfo)
	{
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( filePath);
		
		int fiFlags = mappingFlagsByFormat( fileFMT);

		// 加载获取纹理信息
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load( fileFMT, filePath, fiFlags);
			if( fileHandle)
			{
				imageInfo.m_colorType = FreeImage_GetColorType( fileHandle);
				int	bpp				  = FreeImage_GetBPP( fileHandle);
				if( bpp!=32 && bpp!=24)
					return false;

				if( imageInfo.m_colorType==FIC_RGB && bpp==32)
					imageInfo.m_colorType = FIC_RGBALPHA;

				imageInfo.m_width = FreeImage_GetWidth( fileHandle);
				imageInfo.m_height= FreeImage_GetHeight( fileHandle);

				FreeImage_Unload( fileHandle);

				return true;
			}
		}

		return false;
	}

	bool FreeImageHelper::rescaleImage( const char* iFilePath, const char* oFilePath, float scaleValue)
	{
		std::string ext = PathUtil::GetFileExt( iFilePath).c_str();
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( iFilePath);
		int fiFlags = mappingFlagsByFormat( fileFMT);

		// Load
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* dip = FreeImage_Load( fileFMT, iFilePath, fiFlags);
			if( dip)
			{
				ui32 width = FreeImage_GetWidth( dip);
				ui32 height= FreeImage_GetHeight( dip);

                width  = (ui32)(width * scaleValue);
                height = (ui32)(height* scaleValue);

                // scale
                FIBITMAP* dipScaled = FreeImage_Rescale( dip, width, height, FILTER_BSPLINE);

                FreeImage_Save(fileFMT, dipScaled, oFilePath, TARGA_DEFAULT);

                FreeImage_Unload( dipScaled);
                FreeImage_Unload( dip);

				return true;
			}
		}

		return false;
	}

    bool FreeImageHelper::rescaleImage( const char* iFilePath, const char* oFilePath, ui32 targetWidth, ui32 targetHeight)
    {
        std::string ext = PathUtil::GetFileExt( iFilePath).c_str();
        FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( iFilePath);
        int fiFlags = mappingFlagsByFormat( fileFMT);

        // Load
        if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
        {
            FIBITMAP* dip = FreeImage_Load( fileFMT, iFilePath, fiFlags);
            if( dip)
            {
                FIBITMAP* dipScaled = FreeImage_Rescale( dip, targetWidth, targetHeight, FILTER_BSPLINE);

                FreeImage_Save(fileFMT, dipScaled, oFilePath, TARGA_DEFAULT);

                FreeImage_Unload( dipScaled);
                FreeImage_Unload( dip);

                return true;
            }
        }

        return false;
    }

	bool FreeImageHelper::saveImageToTGA( const char* iFilePath, const char* oFilePath, bool isResale, float scaleValue)
	{
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( iFilePath);

		int fiFlags = mappingFlagsByFormat( fileFMT);
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* dip = FreeImage_Load( fileFMT, iFilePath, fiFlags);
			if( dip)
			{
				ui32 width = FreeImage_GetWidth( dip);
				ui32 height= FreeImage_GetHeight( dip);
				if( isResale)
				{
					width  = (ui32)(width * scaleValue);
					height = (ui32)(height* scaleValue);

					FIBITMAP* dipScaled = FreeImage_Rescale( dip, width, height, FILTER_BOX);

					FreeImage_Save( FIF_TARGA, dipScaled, oFilePath, TARGA_DEFAULT);
					
					FreeImage_Unload( dipScaled);
					FreeImage_Unload( dip);
				}
				else
				{
					FreeImage_Save( FIF_TARGA, dip, oFilePath, TARGA_DEFAULT);

					FreeImage_Unload( dip);
				}

				return true;
			}
		}

		return false;
	}

	bool FreeImageHelper::saveImageToBmp( const char* iFilePath, const char* oFilePath, bool isResale, ui32 width, ui32 height)
	{
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( iFilePath);

		int fiFlags = mappingFlagsByFormat( fileFMT);

		// 加载获取纹理信息
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* dip = FreeImage_Load( fileFMT, iFilePath, fiFlags);
			if( dip)
			{
				if( isResale)
				{
					// 缩放纹理
					FIBITMAP* dipScaled = FreeImage_Rescale( dip, width, height, FILTER_BOX);

					// 若路径不存在,创建路径
					Echo::String saveDir = Echo::PathUtil::GetFileDirPath( oFilePath);
					if( !Echo::PathUtil::IsDirExist( saveDir))
					{
						Echo::PathUtil::CreateDir( saveDir);
					}

					// 执行保存
					FreeImage_Save( FIF_BMP, dipScaled, oFilePath, TARGA_DEFAULT);

					FreeImage_Unload( dipScaled);
					FreeImage_Unload( dip);
				}
				else
				{
					FreeImage_Save( FIF_BMP, dip, oFilePath, TARGA_DEFAULT);

					FreeImage_Unload( dip);
				}

				return true;
			}
		}

		return false;
	}

	bool FreeImageHelper::extracRGBAlphaChannel( const char* srcPath, const char* oRgbFile, const char* oAlphaFile)
	{
		// 存储像素
		BYTE* alphaBits = NULL;
		BYTE* rgbBits   = NULL;

		ImageInfo imageInfo;
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( srcPath);

		int fiFlags = mappingFlagsByFormat( fileFMT);
		 
		// 加载获取纹理信息
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load( fileFMT, srcPath, fiFlags);
			if( fileHandle)
			{
				imageInfo.m_colorType = FreeImage_GetColorType( fileHandle);
				int	bpp				  = FreeImage_GetBPP( fileHandle);
				if( bpp!=32 && bpp!=24)
					return false;

				if( imageInfo.m_colorType==FIC_RGB && bpp==32)
					imageInfo.m_colorType = FIC_RGBALPHA;

				if( imageInfo.m_colorType != FIC_RGBALPHA)
					return false;

				imageInfo.m_width = FreeImage_GetWidth( fileHandle);
				imageInfo.m_height= FreeImage_GetHeight( fileHandle);

				alphaBits	 = new BYTE[imageInfo.m_width*imageInfo.m_height*3];
				rgbBits		 = new BYTE[imageInfo.m_width*imageInfo.m_height*3];
				BYTE* pixels = (BYTE*)FreeImage_GetBits( fileHandle);
				for( size_t pix=0; pix<imageInfo.m_width*imageInfo.m_height; pix++)
				{
					rgbBits[pix*3+0]   = pixels[pix*4+0];
					rgbBits[pix*3+1]   = pixels[pix*4+1];
					rgbBits[pix*3+2]   = pixels[pix*4+2];
					alphaBits[pix*3+0] = pixels[pix*4+3];
					alphaBits[pix*3+1] = pixels[pix*4+3];
					alphaBits[pix*3+2] = pixels[pix*4+3];
				}

				FreeImage_Unload( fileHandle);
			}
		}
		else
		{
			return false;
		}

		saveImageToBmp( rgbBits, imageInfo.m_width, imageInfo.m_height, oRgbFile);
		saveImageToBmp( alphaBits, imageInfo.m_width, imageInfo.m_height, oAlphaFile);

		return true;
	}

	bool FreeImageHelper::extractColors(const char* srcPath, std::vector<Echo::Color>& colors, int& width, int& height)
	{
		colors.clear();

		ImageInfo imageInfo;
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename(srcPath);

		int fiFlags = mappingFlagsByFormat(fileFMT);

		// 加载获取纹理信息
		if (fileFMT != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load(fileFMT, srcPath, fiFlags);
			if (fileHandle)
			{
				imageInfo.m_colorType = FreeImage_GetColorType(fileHandle);
				int	bpp = FreeImage_GetBPP(fileHandle);
				if (bpp != 32 && bpp != 24)
					return false;

				if (imageInfo.m_colorType == FIC_RGB && bpp == 32)
					imageInfo.m_colorType = FIC_RGBALPHA;

				if (imageInfo.m_colorType != FIC_RGBALPHA)
					return false;

				imageInfo.m_width = FreeImage_GetWidth(fileHandle);
				imageInfo.m_height = FreeImage_GetHeight(fileHandle);
				width = imageInfo.m_width;
				height = imageInfo.m_height;

				BYTE* pixels = (BYTE*)FreeImage_GetBits(fileHandle);

				for (size_t h = 0; h < imageInfo.m_height; h++)
				{		
					for (size_t w = 0; w < imageInfo.m_width; w++)
					{
						size_t pix = (imageInfo.m_height - h - 1) * imageInfo.m_width + w;
						BYTE r = pixels[pix * 4 + 0];
						Byte g = pixels[pix * 4 + 1];
						Byte b = pixels[pix * 4 + 2];
						Byte a = pixels[pix * 4 + 3];

						Echo::Color color;
						color.setRGBA(r, g, b, a);

						colors.emplace_back(color);
					}
				}

				FreeImage_Unload(fileHandle);
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	#define GCC_PACK(n)

	int FreeImageHelper::saveImageToBmp( BYTE* pixelData, int width, int height, const char* savePath)
	{
		PathUtil::DelPath( savePath);

		// 保存纹理
#pragma pack (push,1)
		struct BITMAPFILEHEADER
		{
			WORD	bfType GCC_PACK(1);
			DWORD	bfSize GCC_PACK(1);
			WORD	bfReserved1 GCC_PACK(1); 
			WORD	bfReserved2 GCC_PACK(1);
			DWORD	bfOffBits GCC_PACK(1);
		} FH; 
		struct BITMAPINFOHEADER
		{
			DWORD	biSize GCC_PACK(1); 
			i32		biWidth GCC_PACK(1);
			i32		biHeight GCC_PACK(1);
			WORD	biPlanes GCC_PACK(1);
			WORD	biBitCount GCC_PACK(1);
			DWORD	biCompression GCC_PACK(1);
			DWORD	biSizeImage GCC_PACK(1);
			i32		biXPelsPerMeter GCC_PACK(1); 
			i32		biYPelsPerMeter GCC_PACK(1);
			DWORD	biClrUsed GCC_PACK(1);
			DWORD	biClrImportant GCC_PACK(1); 
		} IH;
#pragma pack (pop)

		i32 BytesPerLine = Align( width * 3,4);

		FILE* fileHandle = fopen( savePath, "wb");
		if( !fileHandle)
			return false;

		// File header.
		FH.bfType       		= (WORD) ('B' + 256*'M');
		FH.bfSize       		= (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BytesPerLine * height);
		FH.bfReserved1  		= (WORD) 0;
		FH.bfReserved2  		= (WORD) 0;
		FH.bfOffBits    		= (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

		// Info header.
		IH.biSize               = (DWORD) sizeof(BITMAPINFOHEADER);
		IH.biWidth              = (DWORD) width;
		IH.biHeight             = (DWORD) height;
		IH.biPlanes             = (WORD) 1;
		IH.biBitCount           = (WORD) 24;
		IH.biCompression        = (DWORD) 0; //BI_RGB
		IH.biSizeImage          = (DWORD) BytesPerLine * height;
		IH.biXPelsPerMeter      = (DWORD) 0;
		IH.biYPelsPerMeter      = (DWORD) 0;
		IH.biClrUsed            = (DWORD) 0;
		IH.biClrImportant       = (DWORD) 0;

		// file header
		fwrite( &FH, sizeof(FH), 1, fileHandle);

		// Info header
		fwrite( &IH, sizeof(IH), 1, fileHandle);

		// write pixel data
		fwrite( pixelData, sizeof(BYTE)*width*height*3, 1, fileHandle);

		fflush( fileHandle);
		fclose( fileHandle);

		return true;
	}

	bool FreeImageHelper::getImageBits( const char* filePath,unsigned char* bits )
	{
		ImageInfo imageInfo;
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename( filePath);

		int fiFlags = mappingFlagsByFormat( fileFMT);

		// 加载获取纹理信息
		if( fileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load( fileFMT, filePath, fiFlags);
			if( fileHandle)
			{
				imageInfo.m_colorType = FreeImage_GetColorType( fileHandle);
				int	bpp				  = FreeImage_GetBPP( fileHandle);
				if( bpp!=32 && bpp!=24)
					return false;

				if( imageInfo.m_colorType==FIC_RGB && bpp==32)
					imageInfo.m_colorType = FIC_RGBALPHA;

				imageInfo.m_width = FreeImage_GetWidth( fileHandle);
				imageInfo.m_height= FreeImage_GetHeight( fileHandle);
				BYTE* pixels = (BYTE*)FreeImage_GetBits( fileHandle);

				if( imageInfo.m_colorType==FIC_RGB)
				{
					for( size_t pix=0; pix<imageInfo.m_height; pix++)
					{
						for (size_t pix1=0; pix1<imageInfo.m_width; pix1++)
						{
							bits[(imageInfo.m_width*pix+pix1)*3+0] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*3+2];
							bits[(imageInfo.m_width*pix+pix1)*3+1] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*3+1];
							bits[(imageInfo.m_width*pix+pix1)*3+2] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*3+0];
						}
					}
				}
				else
				{
					for( size_t pix=0; pix<imageInfo.m_height; pix++)
					{
						for (size_t pix1=0; pix1<imageInfo.m_width; pix1++)
						{
							bits[(imageInfo.m_width*pix+pix1)*4+0] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*4+0];
							bits[(imageInfo.m_width*pix+pix1)*4+1] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*4+1];
							bits[(imageInfo.m_width*pix+pix1)*4+2] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*4+2];
							bits[(imageInfo.m_width*pix+pix1)*4+3] = pixels[(imageInfo.m_width*(imageInfo.m_height-pix-1)+pix1)*4+3];
						}
					}
				}

				FreeImage_Unload( fileHandle);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool FreeImageHelper::addWaterMark(const char* dstFile, const char* srcFile)
	{
		FREE_IMAGE_FORMAT dstFileFMT = FreeImage_GetFIFFromFilename( dstFile );
		FREE_IMAGE_FORMAT srcFileFMT = FreeImage_GetFIFFromFilename( srcFile );

		int dstFiFlags = mappingFlagsByFormat( dstFileFMT );
		int srcFiFlags = mappingFlagsByFormat( srcFileFMT );

		// 加载获取纹理信息
		if( dstFileFMT!= FIF_UNKNOWN && FreeImage_FIFSupportsReading( dstFileFMT)
			&& srcFileFMT != FIF_UNKNOWN && FreeImage_FIFSupportsReading(srcFileFMT))
		{
			FIBITMAP* dstFileHandle = FreeImage_Load( dstFileFMT, dstFile, dstFiFlags );
			FIBITMAP* srcFileHandle = FreeImage_Load( srcFileFMT, srcFile, srcFiFlags );
			if( dstFileHandle && srcFileHandle )
			{
				FreeImage_Paste( dstFileHandle, srcFileHandle, 0, 0 , 100 );

// 				int width = FreeImage_GetWidth( dstFileHandle ) / 2;
// 				int height = FreeImage_GetHeight( dstFileHandle ) / 2;
// 				FIBITMAP* reScale = FreeImage_Rescale( dstFileHandle, width, height, FILTER_BOX );

				FIBITMAP* reScale = FreeImage_Rescale( dstFileHandle, 128, 128, FILTER_BOX );

				FreeImage_Save( dstFileFMT, reScale, dstFile );

				FreeImage_Unload( reScale );
				FreeImage_Unload( dstFileHandle );
				FreeImage_Unload( srcFileHandle );

				return true;
			}
		}

		return false;
	}

	bool FreeImageHelper::saveRGBAtoRGBjpeg(const char* srcFile, const char* dstFile, ImageInfo& imageInfo, float scale)
	{
		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename(srcFile);

		int fiFlags = mappingFlagsByFormat(fileFMT);

		// 加载获取纹理信息
		if (fileFMT != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load(fileFMT, srcFile, fiFlags);
			if (fileHandle)
			{
				BITMAPINFO* _PimageInfo = FreeImage_GetInfo(fileHandle);
				if (_PimageInfo->bmiHeader.biBitCount != 32)
					return false;

				//记录原始贴图信息
				imageInfo.m_colorType = FreeImage_GetColorType(fileHandle);
				imageInfo.m_width	  = _PimageInfo->bmiHeader.biWidth;
				imageInfo.m_height	  = _PimageInfo->bmiHeader.biHeight;
				
				if (scale != 1.0f)
				{
					imageInfo.m_width = static_cast<ui32>(imageInfo.m_width*scale);
					imageInfo.m_height = static_cast<ui32>(imageInfo.m_height*scale);
					FIBITMAP* tempHandle = fileHandle;
					fileHandle = FreeImage_Rescale(tempHandle, imageInfo.m_width, imageInfo.m_height, FILTER_BOX);
					FreeImage_Unload(tempHandle);
				}

				size_t  imgSize = imageInfo.m_width*imageInfo.m_height;
				// 缩放纹理
				FIBITMAP* dipScaled = FreeImage_Rescale(fileHandle, imageInfo.m_width, imageInfo.m_height * 2, FILTER_BSPLINE);
				BYTE* pixels = (BYTE*)FreeImage_GetBits(fileHandle);
				BYTE* pixelsScaled = (BYTE*)FreeImage_GetBits(dipScaled);
				// 分离alpha
				for (size_t pix = 0; pix < imgSize; pix++)
				{
					pixelsScaled[pix * 3 + 0] = pixels[pix * 4 + 0];
					pixelsScaled[pix * 3 + 1] = pixels[pix * 4 + 1];
					pixelsScaled[pix * 3 + 2] = pixels[pix * 4 + 2];
					pixelsScaled[imgSize * 3 + pix * 3 + 0] = pixels[pix * 4 + 3];
					pixelsScaled[imgSize * 3 + pix * 3 + 1] = pixels[pix * 4 + 3];
					pixelsScaled[imgSize * 3 + pix * 3 + 2] = pixels[pix * 4 + 3];
				}
				BITMAPINFO* _PimageInfoScaled = FreeImage_GetInfo(dipScaled);
				_PimageInfoScaled->bmiHeader.biBitCount = 24;
				FreeImage_Save(FIF_JPEG, dipScaled, dstFile);
				FreeImage_Unload(fileHandle);
				FreeImage_Unload(dipScaled);
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	bool FreeImageHelper::mergeIMage(StringArray* srcFile, const char* dstFile)
	{
		if ((*srcFile).empty())
			return false;

		FREE_IMAGE_FORMAT fileFMT = FreeImage_GetFIFFromFilename((*srcFile)[0].c_str());

		int fiFlags = mappingFlagsByFormat(fileFMT);

		// 加载获取纹理信息
		if (fileFMT != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fileFMT))
		{
			FIBITMAP* fileHandle = FreeImage_Load(fileFMT, (*srcFile)[0].c_str(), fiFlags);
			if (fileHandle)
			{
				BITMAPINFO* _PimageInfo = FreeImage_GetInfo(fileHandle);
				int textureNum = static_cast<int>((*srcFile).size());
				int texelByte  = int(_PimageInfo->bmiHeader.biBitCount / 8);
				int oldWidth   = _PimageInfo->bmiHeader.biWidth;
				int oldHeight  = _PimageInfo->bmiHeader.biHeight;
				int newHeight  = oldHeight * textureNum;

				// 目标纹理
				FIBITMAP* finalHandle = FreeImage_Rescale(fileHandle, oldWidth, newHeight, FILTER_BSPLINE);
				BYTE* finalPixels = (BYTE*)FreeImage_GetBits(finalHandle);
				BYTE* pixels;
				int imgByte = oldWidth * oldHeight*texelByte;
				FreeImage_Unload(fileHandle);
				for (int i = 0; i < textureNum; ++i)
				{
					fileHandle = FreeImage_Load(fileFMT, (*srcFile)[i].c_str(), fiFlags);
					pixels = (BYTE*)FreeImage_GetBits(fileHandle);
#ifdef ECHO_PLATFORM_WINDOWS
					memcpy_s(finalPixels + imgByte*(textureNum - i -1), oldWidth*newHeight*texelByte, pixels, imgByte);
#else
					memcpy(finalPixels + imgByte * (textureNum - i - 1), pixels, imgByte);
#endif
                    FreeImage_Unload(fileHandle);
				}
					
				FreeImage_Save(FIF_TARGA, finalHandle, dstFile, TARGA_DEFAULT);
				FreeImage_Unload(finalHandle);
			}
		}
		else
		{
			return false;
		}

		return true;
	}

}
