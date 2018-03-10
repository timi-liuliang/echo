#ifndef __ECHO_ENCODER_H__
#define __ECHO_ENCODER_H__

#include "engine/core/Base/EchoDef.h"

#define _ENCODING_LOWER        1
#define _ENCODING_UPPER        2
#define _ENCODING_HALF         4
#define _ENCODING_SIMPLIFIED   8

namespace Echo
{
	ECHO_EXPORT_FOUNDATION void _EncoderInit();

	ECHO_EXPORT_FOUNDATION void _NormalizeUTF8(char* text, unsigned options);
	ECHO_EXPORT_FOUNDATION void _NormalizeGBK(char* text, unsigned options);

	ECHO_EXPORT_FOUNDATION void _GBK_TO_UTF8(const char* from, unsigned int from_len, char** to, unsigned int* to_len);
	ECHO_EXPORT_FOUNDATION void _UTF8_TO_GBK(const char* from, unsigned int from_len, char** to, unsigned int* to_len);
}


#endif