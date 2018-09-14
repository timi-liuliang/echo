#pragma once

#include "engine/core/math/Math.h"

namespace nn
{
	// sigmoid : activation function
	INLINE float sigmoid( float x)
	{
		// final sigmoid value
		return 1.f / (1.f + exp(-x));
	}
}