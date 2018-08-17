#include "FSAudioBase.h"
#include "engine/core/log/Log.h"
#include <stdarg.h>

namespace Echo
{
	// FMODCheckError
	void FSErrorCheck(FMOD_RESULT result, const char* file, int line, const char* formats, ...)
	{
		if( result!=FMOD_OK)
		{
			// ∂ÓÕ‚¥ÌŒÛ–≈œ¢
			if (formats)
			{
				char szBuffer[4096];
				int bufferLength = sizeof(szBuffer);
				va_list args;
				va_start(args, formats);
				vsnprintf(szBuffer, bufferLength, formats, args);
				va_end(args);

				szBuffer[bufferLength - 1] = 0;

				EchoLogError("FMOD Studio Error[%d-%s], File[%s] Line[%d]", result, szBuffer, file, line);
			}
			else
			{
				EchoLogError("FMOD Studio Error[%d], File[%s] Line[%d]", result, file, line);
			}
		}
	}
}