/*=============================================================================
	ContainerAllocationPolicies.cpp: Container allocation policy implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
#include "stdafx.h"
#include "LMCore.h"

namespace Lightmass
{

/** Whether array slack is being tracked. */
#define TRACK_ARRAY_SLACK 0

#if TRACK_ARRAY_SLACK
	FStackTracker* GSlackTracker = NULL;
#endif

SIZE_T DefaultCalculateSlack(SIZE_T NumElements,SIZE_T NumAllocatedElements,SIZE_T BytesPerElement)
{
	#if TRACK_ARRAY_SLACK 
		if( !GSlackTracker )
		{
			GSlackTracker = new FStackTracker();
		}
		#define SLACK_TRACE_TO_SKIP 4
		GSlackTracker->CaptureStackTrace(SLACK_TRACE_TO_SKIP);
	#endif

	if(NumElements < NumAllocatedElements)
	{
		// If the container has too much slack, shrink it to exactly fit the number of elements.
		const SIZE_T CurrentSlackElements = NumAllocatedElements-NumElements;
		const SIZE_T CurrentSlackBytes = (NumAllocatedElements-NumElements)*BytesPerElement;
		const UBOOL bTooManySlackBytes = CurrentSlackBytes >= 1024 * 1024;
		const UBOOL bTooManySlackElements = 3*NumElements < 2*NumAllocatedElements;
		if(	(bTooManySlackBytes || bTooManySlackElements) && (CurrentSlackElements > 64 || !NumElements) )
		{
			return NumElements;
		}
		else
		{
			return NumAllocatedElements;
		}
	}
	else if(NumElements > 0)
	{
		// Allocate slack for the array proportional to its size.
		return NumElements + 3*NumElements/8 + 16;
	}
	else
	{
		return 0;
	}
}

}