#pragma once

#ifndef EVENT_CONFIG_H
#define EVENT_CONFIG_H

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(_WIN64) || defined(__WIN64__) || defined(WIN64)
	#include "event-config-win32.h"
#elif defined(__APPLE_CC__)
	#include "event-config-iOS.h"
#elif defined(__ANDROID__)
	#include "event-config-android.h"
#elif defined(__EMSCRIPTEN__)
	#include "event-config-html5.h"
#endif

#endif