#pragma once

#include "decoder_base.h"
#include "de265.h"

namespace cmpeg
{
	class decoder_mpeg5 : public decoder_base
	{
	public:
		decoder_mpeg5();
		virtual ~decoder_mpeg5();
		virtual bool write(int pts, const std::vector<uint8_t>& buffer);
		virtual bool decode();

	private:
		de265_decoder_context*			m_ctx;		// 解码器上下文
	};
}