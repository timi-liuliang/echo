#pragma once

#include <engine/core/base/echo_def.h>

#if defined(ECHO_PLATFORM_IOS) || defined(ECHO_PLATFORM_MAC) || defined(ECHO_PLATFORM_ANDROID) || defined(ECHO_PLATFORM_LINUX)
	#define NDEBUG
#endif

#include <PxPhysicsAPI.h>
#include <vehicle/PxVehicleSDK.h>
