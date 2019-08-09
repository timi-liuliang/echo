#include "vk_gpu_buffer.h"
#include "vk_mapping.h"
#include "vk_renderer.h"

namespace Echo
{
	VKBuffer::VKBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
    {
		updateData(buff);
    }

    VKBuffer::~VKBuffer()
    {
        clear();
    }

    bool VKBuffer::updateData(const Buffer& buff)
    {
        clear();

        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        VkBufferCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.size = buff.getSize();
        createInfo.usage = VKMapping::MapGpuBufferType(m_type);
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.flags = 0;
        
        if (VK_SUCCESS == vkCreateBuffer(vkRenderer->getVkDevice(), &createInfo, nullptr, &m_vkBuffer))
        {
            // memory requirements
            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(vkRenderer->getVkDevice(), m_vkBuffer, &memRequirements);

            return true;
        }

        EchoLogError("vulkan crete gpu buffer failed");
        return false;
    }

    void VKBuffer::bindBuffer()
    {

    }

    void VKBuffer::clear()
    {
        if (m_vkBuffer)
        {
            VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());
            vkDestroyBuffer(vkRenderer->getVkDevice(), m_vkBuffer, nullptr);
            m_vkBuffer = nullptr;
        }
    }
}
