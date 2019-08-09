#pragma once

#include <engine/core/render/interface/GPUBuffer.h>
#include "vk_render_base.h"

namespace Echo
{
    class VKBuffer : public GPUBuffer
    {
    public:
        VKBuffer(GPUBufferType type, Dword usage, const Buffer& buff);
        ~VKBuffer();

        bool updateData(const Buffer& buff);
        void bindBuffer();

        // clear
        void clear();

    private:
        VkBuffer    m_vkBuffer = nullptr;
	};
}
