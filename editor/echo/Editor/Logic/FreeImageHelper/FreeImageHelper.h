#pragma once

#include <engine/core/base/type_def.h>
#include <freeimage/FreeImage.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/math/color.h>

namespace Echo
{
	class FreeImageHelper
	{
	public:
		struct ImageInfo
		{
			ui32					m_width;		// 宽
			ui32					m_height;		// 高
			FREE_IMAGE_COLOR_TYPE	m_colorType;	// 格式类型
		};

	public:
		FreeImageHelper();
		~FreeImageHelper();
        
        // instance
        static FreeImageHelper* instance();

		/** 获取图像信息 */
        bool getImageInfo( const char* filePath, ImageInfo& imageInfo);

		// scale image
        bool rescaleImage( const char* iFilePath, const char* oFilePath, float scaleValue=1.f);
        bool rescaleImage( const char* iFilePath, const char* oFilePath, ui32 targetWidth, ui32 targetHeight);

		/** 保存为tga格式，无压缩 */
        bool saveImageToTGA( const char* iFilePath, const char* oFilePath, bool isResale=false, float scaleValue=1.f);

		/** 保存为bmp格式，无压缩 */
        bool saveImageToBmp( const char* iFilePath, const char* oFilePath, bool isResale, ui32 width, ui32 height);

		/** 提取alpha通道到一个新文件 */
        bool extracRGBAlphaChannel( const char* srcPath, const char* oRgbFile, const char* oAlphaFile);

		/** 映射标记 */
        int MappingFlagsByFormat( FREE_IMAGE_FORMAT fmt);

		/** 保存为bmp格式(仅支持RGB格式) */
        int SaveImageToBmp( BYTE* pixelData, int widht, int height, const char* savePath);

		/** 获取图片像素buffer */
        bool getImageBits(const char* filePath,unsigned char* bits);

		/** 添加水印 */
        bool	addWaterMark(const char* dstFile, const char* srcFile);

		/** 将alpha通道分解到RGB并添加到末尾 */
        bool SaveRGBAtoRGBjpeg(const char* srcFile, const char* dstFile, ImageInfo& imageInfo,float scale);

		/** 合并贴图宽不变高度增加（输出tga） */
        bool mergeIMage(StringArray* srcFile, const char* dstFile);

		/** 从文件中提取颜色值 */
        bool extractColors(const char* srcPath, std::vector<Echo::Color>& colors, int& width, int& height);
	};
}
