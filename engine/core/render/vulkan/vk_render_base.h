#pragma once

#include <engine/core/log/Log.h>

#ifdef ECHO_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(ECHO_PLATFORM_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#ifdef ECHO_PLATFORM_ANDROID
    #include <vulkan/vulkan.h>
    #include <vulkan/vk_platform.h>
    #include <vulkan/vulkan_android.h>
    #include <thirdparty/spirv-cross/spirv_cross.hpp>
#else
    #include <vulkan/vulkan.h>
    #include <vulkan/vk_sdk_platform.h>
    #include <thirdparty/spirv-cross/spirv_cross.hpp>
#endif

namespace Echo
{
    // Debug Vk Error
    void OutputVKError(VkResult vkResult, const char* filename, int lineNum);
}

#ifdef ECHO_DEBUG
#define VKDebug(Func) { VkResult result = Func; Echo::OutputVKError( result, __FILE__, __LINE__ ); }
#else
#define VKDebug(Func)  Func;
#endif