/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_PARAMETERS_TYPES_H
#define NX_PARAMETERS_TYPES_H

namespace NxParameterized
{

struct DummyDynamicArrayStruct
{
	void *buf;
	bool isAllocated;
	int elementSize;
	int arraySizes[1];
};

struct DummyStringStruct
{
	const char *buf;
	bool isAllocated;

	//This is needed for compatibility with existing code
	operator const char *() const
	{
		return buf;
	}
};

class Interface; // necessary for NX_PARAMETERIZED_ONLY_LAYOUTS

}

#endif
