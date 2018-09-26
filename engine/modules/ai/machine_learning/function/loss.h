#pragma once

#include "engine/core/math/Math.h"
#include "engine/core/math/matrix.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"

namespace nn
{
	// squared error function 1/2 * (YStart - Y) * (YStar - Y)
	// https://www.khanacademy.org/math/statistics-probability/describing-relationships-quantitative-data/more-on-regression/v/squared-error-of-regression-line
	Echo::Matrix squaredError(const Echo::Matrix& output, const Echo::Matrix& desiredOutput)
	{
		Echo::Matrix squaredError( output.getHeight(), output.getWidth());
		if (output.getWidth() == desiredOutput.getWidth() && output.getHeight() == desiredOutput.getHeight())
		{
			for (int i = 0; i < output.getHeight(); i++)
			{
				for (int j = 0; j < output.getWidth(); j++)
				{
					float error = desiredOutput[i][j] - output[i][j];
					squaredError[i][j] = error * error;
				}
			}
		}
		else
		{
			EchoLogError("calculate square error failed, elements number is different between output and desired output.");
		}

		return squaredError;
	}

	INLINE Echo::Matrix squaredErrorPrime(const Echo::Matrix& output, const Echo::Matrix& desiredOutput)
	{
		Echo::Matrix squaredError(output.getHeight(), output.getWidth());
		if (output.getWidth() == desiredOutput.getWidth() && output.getHeight() == desiredOutput.getHeight())
		{
			for (int i = 0; i < output.getHeight(); i++)
			{
				for (int j = 0; j < output.getWidth(); j++)
				{
					float error = desiredOutput[i][j] - output[i][j];
					squaredError[i][j] = 2.f * error;
				}
			}
		}
		else
		{
			EchoLogError("calculate square error failed, elements number is different between output and desired output.");
		}

		return squaredError;
	}
}