#pragma once

#include "engine/core/base/type_def.h"
#include "engine/core/math/Math.h"

namespace nn
{
	// sigmoid : activation function
	INLINE float sigmoid(float x)
	{
		// final sigmoid value
		return 1.f / (1.f + exp(-x));
	}

	INLINE float sigmoid_prime(float x)
	{
		// https://math.stackexchange.com/questions/78575/derivative-of-sigmoid-function-sigma-x-frac11e-x
		float sm = sigmoid(x);
		return sm * (1.f - sm);
	}

	INLINE float random(float x)
	{
		return (float)(rand() % 10000 + 1) / 10000.f - 0.5f;
	}
}