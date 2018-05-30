#pragma once

#include "engine/core/Math/Math.h"
#include <fmod_studio.hpp>
#include <fmod.hpp>

// ÉùÃ÷
namespace FMOD
{
	class System;
	namespace Studio
	{ 
		class System; 
		class Bank;
		class EventDescription;
		class EventInstance;
	}
}

namespace Echo
{
	typedef ui32 AudioSourceID;

	// FMODCheckError
	void FSErrorCheck(FMOD_RESULT result, const char* file, int line, const char* formats=nullptr, ...);
}

#define FS_ERRORCHECK( result, formats, ...) Echo::FSErrorCheck( result, __FILE__, __LINE__, formats, ##__VA_ARGS__)
