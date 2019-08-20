#pragma once

#include <engine/core/log/Log.h>

#ifdef ECHO_PLATFORM_WINDOWS
	#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(ECHO_PLATFORM_ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>