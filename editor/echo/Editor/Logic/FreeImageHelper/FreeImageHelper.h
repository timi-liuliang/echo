#pragma once

#include <engine/core/Base/type_def.h>
#include <freeimage/FreeImage.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/math/color.h>

namespace Echo
{
	/**
	 * FreeImage库封装 2014-6-25
	 */
	class FreeImageHelper
	{
	public:
		/** 图像信息 */
		struct ImageInfo
		{
			ui32					m_width;		// 宽
			ui32					m_height;		// 高
			FREE_IMAGE_COLOR_TYPE	m_colorType;	// 格式类型
		};

	public:
		FreeImageHelper();
		~FreeImageHelper();

		/** 获取图像信息 */
		static bool getImageInfo( const char* filePath, ImageInfo& imageInfo);

		/** 重新缩放纹理 */
		static bool rescaleImage( const char* iFilePath, const char* oFilePath, bool isResale=false, float scaleValue=1.f);

		/** 保存为tga格式，无压缩 */
		static bool saveImageToTGA( const char* iFilePath, const char* oFilePath, bool isResale=false, float scaleValue=1.f);

		/** 保存为bmp格式，无压缩 */
		static bool saveImageToBmp( const char* iFilePath, const char* oFilePath, bool isResale, ui32 width, ui32 height);

		/** 提取alpha通道到一个新文件 */
		static bool extracRGBAlphaChannel( const char* srcPath, const char* oRgbFile, const char* oAlphaFile);

		/** 映射标记 */
		static int MappingFlagsByFormat( FREE_IMAGE_FORMAT fmt);

		/** 保存为bmp格式(仅支持RGB格式) */
		static int SaveImageToBmp( BYTE* pixelData, int widht, int height, const char* savePath);

		/** 获取图片像素buffer */
		static bool getImageBits(const char* filePath,unsigned char* bits);

		/** 添加水印 */
		static bool	addWaterMark(const char* dstFile, const char* srcFile);

		/** 将alpha通道分解到RGB并添加到末尾 */
		static bool SaveRGBAtoRGBjpeg(const char* srcFile, const char* dstFile, ImageInfo& imageInfo,float scale);

		/** 合并贴图宽不变高度增加（输出tga） */
		static bool mergeIMage(StringArray* srcFile, const char* dstFile);

		/** 从文件中提取颜色值 */
		static bool extractColors(const char* srcPath, std::vector<Echo::Color>& colors, int& width, int& height);
	};
}