/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "foundation/PxErrorCallback.h"

#include <NxApexDefs.h>

#if NX_SDK_VERSION_MAJOR == 2

#include "NxUserOutputStream.h"

class SimpleNxErrorStream : public NxUserOutputStream
{
public:
	virtual void reportError(NxErrorCode code, const char* message, const char* file, int line)
	{
		const char* errorCode = NULL;
		switch (code)
		{
		case NXE_NO_ERROR: return;

#define CASE(_A) case _A: errorCode = #_A; break;
			CASE(NXE_INVALID_PARAMETER)
			CASE(NXE_INVALID_OPERATION)
			CASE(NXE_OUT_OF_MEMORY)
			CASE(NXE_INTERNAL_ERROR)
			CASE(NXE_ASSERTION)
			CASE(NXE_DB_INFO)
			CASE(NXE_DB_WARNING)
			CASE(NXE_DB_PRINT)
#undef CASE
		}

		if (errorCode != NULL)
		{
			printf("PhysX error: %s in %s:%d\n%s\n", errorCode, file, line, message);
		}
	}

	virtual NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
	{
		printf("Assert Violation in %s:%d\n%s\n", file, line, message);
		return NX_AR_BREAKPOINT;
	}

	virtual void print(const char* message)
	{
		printf(message);
	}
};

#endif // NX_SDK_VERSION_MAJOR == 2

class SimplePxErrorStream : public physx::PxErrorCallback
{
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
	{
		const char* errorCode = NULL;
		switch (code)
		{
		case physx::PxErrorCode::eNO_ERROR :
			return;
		case physx::PxErrorCode::eINVALID_PARAMETER:
			errorCode = "Invalid Parameter";
			break;
		case physx::PxErrorCode::eINVALID_OPERATION:
			errorCode = "Invalid Operation";
			break;
		case physx::PxErrorCode::eOUT_OF_MEMORY:
			errorCode = "Out of Memory";
			break;
		case physx::PxErrorCode::eINTERNAL_ERROR :
			errorCode = "Internal Error";
			break;
//		case physx::PxErrorCode::eASSERTION:
//			errorCode = "Assertion";
//			break;
		case physx::PxErrorCode::eDEBUG_INFO:
			errorCode = "Debug Info";
			break;
		case physx::PxErrorCode::eDEBUG_WARNING:
			errorCode = "Debug Warning";
			break;
//		case physx::PxErrorCode::eSERIALIZATION_ERROR:
//			errorCode = "Serialization Error";
//			break;
		default:
			errorCode = "Unknown Error Code";
		}

		if (errorCode != NULL)
		{
			printf("PhysX error: %s %s:%d\n%s\n", errorCode, file, line, message);
		}
		else
		{
			printf("PhysX error: PxErrorCode is %d in %s:%d\n%s\n", code, file, line, message);
		}
	}

	virtual void print(const char* message)
	{
		printf("%s", message);
	}
};
