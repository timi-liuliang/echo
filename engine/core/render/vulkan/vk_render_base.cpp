#include "vk_render_base.h"

namespace Echo
{
    void OutputVKError(VkResult vkResult, const char* filename, int lineNum)
    {
        if (vkResult != VK_SUCCESS)
        {
            switch (vkResult)
            {
            case VK_NOT_READY:              EchoLogError("File Line Number[%s:%d] Vulkan not ready.", filename, lineNum);           break;
            case VK_ERROR_DEVICE_LOST:      EchoLogError("File Line Number[%s:%d] Vulkan device lost error", filename, lineNum);    break;
            case VK_ERROR_OUT_OF_DATE_KHR:  EchoLogError("File Line Number[%s:%d] Vulkan error out of date khr", filename, lineNum);break;
            default:                        EchoLogError("File Line Number[%s:%d] Vulkan unknown error.", filename, lineNum);       break;
            }
        }

    }
}