/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "SnConvX.h"
#include <stdarg.h>
#include "PxErrorCallback.h"
#include "PsString.h"

#define MAX_DISPLAYED_ISSUES   10

using namespace physx;

void Sn::ConvX::resetNbErrors()
{
	mNbErrors = 0;
	mNbWarnings = 0;
}

int Sn::ConvX::getNbErrors() const
{
	return mNbErrors;
}

void Sn::ConvX::displayMessage(PxErrorCode::Enum code, const char* format, ...)
{
	if(silentMode())
		return;
		
	int sum = mNbWarnings + mNbErrors;
	if(sum >= MAX_DISPLAYED_ISSUES)
		return;

	bool display = false;

	if(code==PxErrorCode::eINTERNAL_ERROR || code==PxErrorCode::eINVALID_OPERATION || code==PxErrorCode::eINVALID_PARAMETER)
	{
		mNbErrors++;
		display = true;
	}
	else if(code == PxErrorCode::eDEBUG_WARNING)
	{
		mNbWarnings++;	
		display = true;
	}

	if(display || ((sum == 0) && verboseMode()) )
	{
		va_list va;
		va_start(va, format);
		Ps::getFoundation().errorImpl(code, __FILE__, __LINE__, format, va);
		va_end(va); 
	}

	if(display)
	{
		if( sum == 0)
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "Hit warnings or errors: skipping further verbose output.\n");		    
		}
		else if(sum == MAX_DISPLAYED_ISSUES-1)
		{
			Ps::getFoundation().error(PxErrorCode::eDEBUG_INFO, __FILE__, __LINE__, "Exceeding 10 warnings or errors: skipping further output.\n");		    
		}
	}

	return;
}
