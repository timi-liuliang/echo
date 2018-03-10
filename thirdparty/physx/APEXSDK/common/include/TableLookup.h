/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef TABLE_LOOKUP_H
#define TABLE_LOOKUP_H

namespace physx
{
namespace apex
{

// Stored Tables
const PxF32 custom[] =	{ 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f, 1.00f };
const PxF32 linear[] =	{ 1.00f, 0.90f, 0.80f, 0.70f, 0.60f, 0.50f, 0.40f, 0.30f, 0.20f, 0.10f, 0.00f };
const PxF32 steep[] =	{ 1.00f, 0.99f, 0.96f, 0.91f, 0.84f, 0.75f, 0.64f, 0.51f, 0.36f, 0.19f, 0.00f };
const PxF32 scurve[] =	{ 1.00f, 0.99f, 0.96f, 0.91f, 0.80f, 0.50f, 0.20f, 0.09f, 0.04f, 0.01f, 0.00f };

// Table sizes must be same for all stored tables
#define NUM_ELEMENTS(X) (sizeof(X)/sizeof(*(X)))
#define TABLE_SIZE NUM_ELEMENTS(custom)

// Stored Table enums
struct TableName
{
	enum Enum
	{
		CUSTOM = 0,
		LINEAR,
		STEEP,
		SCURVE,
	};
};

struct TableLookup
{
	PxF32 xVals[TABLE_SIZE];
	PxF32 yVals[TABLE_SIZE];
	PxF32 x1;
	PxF32 x2;
	PxF32 multiplier;

#ifdef __CUDACC__
	__device__ TableLookup() {}
#else
	TableLookup():
		x1(0),
		x2(0),
		multiplier(0)
	{
		zeroTable();
	}
#endif

	PX_CUDA_CALLABLE void zeroTable()
	{
		for (size_t i = 0; i < TABLE_SIZE; ++i)
		{
			xVals[i] = 0.0f;
			yVals[i] = 0.0f;
		}
	}

	PX_CUDA_CALLABLE void applyStoredTable(TableName::Enum tableName)
	{
		// build y values
		for (size_t i = 0; i < TABLE_SIZE; ++i)
		{
			if (tableName == TableName::LINEAR)
			{
				yVals[i] = linear[i];
			}
			else if (tableName == TableName::STEEP)
			{
				yVals[i] = steep[i];
			}
			else if (tableName == TableName::SCURVE)
			{
				yVals[i] = scurve[i];
			}
			else if (tableName == TableName::CUSTOM)
			{
				yVals[i] = custom[i];
			}
		}
	}

	PX_CUDA_CALLABLE void buildTable()
	{
		// build x values
		PxF32 interval = (x2 - x1) / (TABLE_SIZE - 1);
		for (size_t i = 0; i < TABLE_SIZE; ++i)
		{
			xVals[i] = x1 + i * interval;
		}

		// apply multipler to y values
		if (multiplier >= -1.0f && multiplier <= 1.0f)
		{
			for (size_t i = 0; i < TABLE_SIZE; ++i)
			{
				yVals[i] = yVals[i] * multiplier;
			}

			// offset = max y value in array
			PxF32 max = yVals[0];
			for (size_t i = 1; i < TABLE_SIZE; ++i)
			{
				if (yVals[i] > max)
				{
					max = yVals[i];
				}
			}

			// apply offset
			for (size_t i = 0; i < TABLE_SIZE; ++i)
			{
				yVals[i] = yVals[i] + (1.0f - max);
			}
		}
	}
	
	PX_CUDA_CALLABLE PxF32 lookupTableValue(PxF32 x) const
	{
		if (x <= xVals[0])
		{
			return yVals[0];
		}
		else if (x >= xVals[TABLE_SIZE-1])
		{
			return yVals[TABLE_SIZE-1];
		}
		else
		{
			// linear interpolation between x values
			physx::PxF32 interval = (xVals[TABLE_SIZE-1] - xVals[0]) / (TABLE_SIZE - 1);
			physx::PxU32 lerpPos = (PxU32)((x - xVals[0]) / interval);
			physx::PxF32 yDiff = yVals[lerpPos+1] - yVals[lerpPos];
			return yVals[lerpPos] + (x - xVals[lerpPos]) / interval * yDiff;
		}
	}
};

}
} // namespace physx::apex

#endif
