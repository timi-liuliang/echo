#pragma once

#include <stdint.h>

namespace cmpeg
{
	class render_base
	{
	public:
		virtual ~render_base() {}
		virtual void resize(int widht, int height) = 0;
		virtual void render(const uint8_t* Y, const uint8_t* Cr, const uint8_t* Cb) = 0;
	};
}