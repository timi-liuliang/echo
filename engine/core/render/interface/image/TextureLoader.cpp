#pragma once

#include "TextureLoader.h"

namespace Echo
{
	PixelFormat pvrformatMapping( ui64& pixelFormat)
	{
		switch( pixelFormat )
		{
		case ePVRTPF_PVRTCI_2bpp_RGB:
			return PF_PVRTC1_2bpp_RGB;

		case ePVRTPF_PVRTCI_2bpp_RGBA:
			return PF_PVRTC1_2bpp_RGBA;

		case ePVRTPF_PVRTCI_4bpp_RGB:
			return PF_PVRTC1_4bpp_RGB;

		case ePVRTPF_PVRTCI_4bpp_RGBA:
			return PF_PVRTC1_4bpp_RGBA;

		case ePVRTPF_ETC1:
			return PF_ETC1; 

		case ePVRTPF_ETC2_RGB:
			return PF_ETC2_RGB;

		case ePVRTPF_ETC2_RGBA:
			return PF_ETC2_RGBA;

		case ePVRTPF_RGBA_4444:
			return PF_PVRTC_RGBA_4444;
		}

		return PF_UNKNOWN; 
	}

	// PVR 像素分量
	void pvrPixelDimension( ui64 pixelFormat, ui32& xDimension, ui32& yDimension, ui32& zDimension )
	{
		switch( pixelFormat )
		{
		case ePVRTPF_DXT1:
		case ePVRTPF_DXT2:
		case ePVRTPF_DXT3:
		case ePVRTPF_DXT4:
		case ePVRTPF_DXT5:
		case ePVRTPF_BC4:
		case ePVRTPF_BC5:
		case ePVRTPF_ETC1:
		case ePVRTPF_ETC2_RGB:
		case ePVRTPF_ETC2_RGBA:
		case ePVRTPF_ETC2_RGB_A1:
		case ePVRTPF_EAC_R11:
		case ePVRTPF_EAC_RG11:
		case ePVRTPF_RGBA_4444:
			xDimension = 4;
			yDimension = 4;
			zDimension = 1;
			return ;
		case ePVRTPF_PVRTCI_4bpp_RGB:
		case ePVRTPF_PVRTCI_4bpp_RGBA:
			xDimension = 8;
			yDimension = 8;
			zDimension = 1;
			return ;
		case ePVRTPF_PVRTCI_2bpp_RGB:
		case ePVRTPF_PVRTCI_2bpp_RGBA:
			xDimension = 16;
			yDimension = 8;
			zDimension = 1;
			return ;
		case ePVRTPF_PVRTCII_4bpp:
			xDimension = 4;
			yDimension = 4;
			zDimension = 1;
			return ;
		case ePVRTPF_PVRTCII_2bpp:
			xDimension = 8;
			yDimension = 4;
			zDimension = 1;
			return ;
		case ePVRTPF_UYVY:
		case ePVRTPF_YUY2:
		case ePVRTPF_RGBG8888:
		case ePVRTPF_GRGB8888:
			xDimension = 2;
			yDimension = 1;
			zDimension = 1;
			return ;
		case ePVRTPF_BW1bpp:
			xDimension = 8;
			yDimension = 1;
			zDimension = 1;
			return ;
		default: //Non-compressed formats all return 1.
			xDimension = 1;
			yDimension = 1;
			zDimension = 1;
			return ;
		}

		xDimension = 0;
		yDimension = 0;
		zDimension = 0;
		return ;
	}

	// 像素位数映射
	ui32 pvrBitsPerPixel( ui64 pixelFormat )
	{
		switch ( pixelFormat )
		{
		case ePVRTPF_BW1bpp:
			return 1;
		case ePVRTPF_PVRTCI_2bpp_RGB:
		case ePVRTPF_PVRTCI_2bpp_RGBA:
		case ePVRTPF_PVRTCII_2bpp:
			return 2;
		case ePVRTPF_PVRTCI_4bpp_RGB:
		case ePVRTPF_PVRTCI_4bpp_RGBA:
		case ePVRTPF_PVRTCII_4bpp:
		case ePVRTPF_ETC1:
		case ePVRTPF_EAC_R11:
		case ePVRTPF_ETC2_RGB:	
		case ePVRTPF_ETC2_RGB_A1:
		case ePVRTPF_DXT1:
		case ePVRTPF_BC4:
		case ePVRTPF_RGBA_4444:
			return 4;
		case ePVRTPF_DXT2:
		case ePVRTPF_DXT3:
		case ePVRTPF_DXT4:
		case ePVRTPF_DXT5:
		case ePVRTPF_BC5:
		case ePVRTPF_EAC_RG11:
		case ePVRTPF_ETC2_RGBA:
			return 8;
		case ePVRTPF_YUY2:
		case ePVRTPF_UYVY:
		case ePVRTPF_RGBG8888:
		case ePVRTPF_GRGB8888:
			return 16;
		case ePVRTPF_SharedExponentR9G9B9E5:
			return 32;
		case ePVRTPF_NumCompressedPFs:
			return 0;
		}

		return 0;
	}
}