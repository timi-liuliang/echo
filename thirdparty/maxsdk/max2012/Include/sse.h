/////////////////////////////////////////////////////////////////////////
//
//
//	SSE (Streaming SIMD extentions)
//
//	Created 9/15/2001	Claude Robillard
//

#pragma once
#include "maxheap.h"
#include <WTypes.h>

//////////////////////////////////////////////////////////////
//
//		IRendSSEControl compatibility interface
//
//
class IRenderSSEControl: public MaxHeapOperators
{
public:
	// this interface's ID
	enum { IID = 0xcafdface };

	// This function returns TRUE when user whishes to use SSE for rendering optimizations
	virtual BOOL IsSSEEnabled() = 0;
};



