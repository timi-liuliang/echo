#pragma once

#include "engine/core/math/Math.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "../math/matrix.h"

namespace nn
{
	// squared error function
	// https://www.khanacademy.org/math/statistics-probability/describing-relationships-quantitative-data/more-on-regression/v/squared-error-of-regression-line
	INLINE float squaredError(Matrix& output, Matrix& desiredOutput)
	{
		float squaredError = 0.f;
		if (output.getNumberElements() == desiredOutput.getNumberElements() && output.getNumberElements()>0)
		{
			for (int i = 0; i < output.getNumberElements(); i++)
			{
				float error = desiredOutput[i] - output[i];
				squaredError += error * error;
			}
		}
		else
		{
			EchoLogError("calculate square error failed, elements number is different between output and desired output.");
		}

		return squaredError;
	}

	INLINE float squaredErrorPrime()
	{

	}
}