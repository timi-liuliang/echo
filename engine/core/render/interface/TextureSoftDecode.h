#ifndef __TEXTURE_SOFT_DECODE_H__
#define __TEXTURE_SOFT_DECODE_H__

#include "engine/core/Base/EchoDef.h"

namespace Echo
{
	enum DECODE_CODEC { CODEC_ETC, CODEC_ETC2 };

	struct DecodeInfo
	{
		i32 m_format = 0;
		i32 m_codec = CODEC_ETC2;
		i32 m_signed = 0;
		i32 m_width = 0;
		i32 m_height = 0;
		i32 m_activeWidth = 0;
		i32 m_activeHeight = 0;
		bool m_flipY = false;
	};

	class TextureSoftDecode
	{
	public:
		static ui8* decodeKTX(Byte* &data);

		static ui8* decodePVR(Byte* &data);

		static ui8* decodeData(ui8* data, const DecodeInfo& decodeInfo);

		static ui8* assembleData(ui8* img, ui8* alpha, const DecodeInfo& decodeInfo);

		static void setSoftDecode(bool state) { m_isSoftDecode = state; }

		static bool isSoftDecode() { return m_isSoftDecode; }

	public:
		static bool m_isSoftDecode;
	};
}

#endif
