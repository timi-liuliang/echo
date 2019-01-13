#pragma once

#include "engine/core/render/interface/FrameBuffer.h"

namespace Echo
{
	class VKFrameBuffer : public FrameBuffer
	{
	public:
		VKFrameBuffer();
		virtual ~VKFrameBuffer();
	};
}
