#pragma once

#include <engine/core/render/interface/GPUBuffer.h>
#include "mt_render_base.h"

namespace Echo
{
	class MTBuffer: public GPUBuffer
	{
	public:
		MTBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
		~MTBuffer();

		bool updateData(const Buffer& buff);
        void bindBuffer();
        
    public:
        // get metal buffer
        id<MTLBuffer> getMetalBuffer() { return m_metalBuffer; }

	private:
        id<MTLBuffer>       m_metalBuffer;
	};
}
