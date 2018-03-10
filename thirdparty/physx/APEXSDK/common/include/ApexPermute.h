/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_PERMUTE_H
#define APEX_PERMUTE_H

namespace physx
{
namespace apex
{

// permutationBuffer has to contain the indices that map from the new to the old index
template<class Sortable>
inline void ApexPermute(Sortable* sortBuffer, const PxU32* permutationBuffer, PxU32 numElements, PxU32 numElementsPerPermutation = 1)
{
	shdfnd::Array<Sortable> temp;
	temp.resize(numElementsPerPermutation);

	// TODO remove used buffer
	shdfnd::Array<bool> used(numElements, false);

	for (PxU32 i = 0; i < numElements; i++)
	{
		//if (permutationBuffer[i] == (PxU32)-1 || permutationBuffer[i] == i)
		if (used[i] || permutationBuffer[i] == i)
		{
			continue;
		}

		PxU32 dst = i;
		PxU32 src = permutationBuffer[i];
		for (PxU32 j = 0; j < numElementsPerPermutation; j++)
		{
			temp[j] = sortBuffer[numElementsPerPermutation * dst + j];
		}
		do
		{
			for (PxU32 j = 0; j < numElementsPerPermutation; j++)
			{
				sortBuffer[numElementsPerPermutation * dst + j] = sortBuffer[numElementsPerPermutation * src + j];
			}
			//permutationBuffer[dst] = (PxU32)-1;
			used[dst] = true;
			dst = src;
			src = permutationBuffer[src];
			//} while (permutationBuffer[src] != (PxU32)-1);
		}
		while (!used[src]);
		for (PxU32 j = 0; j < numElementsPerPermutation; j++)
		{
			sortBuffer[numElementsPerPermutation * dst + j] = temp[j];
		}
		//permutationBuffer[dst] = (PxU32)-1;
		used[dst] = true;
	}
}

} // namespace apex
} // namespace physx

#endif // APEX_PERMUTE_H
