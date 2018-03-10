/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PX_UNION_H
#define PX_UNION_H

namespace physx { namespace Sn {
	struct UnionType
	{
		const char*			mTypeName;
		int					mTypeValue;
	};

	struct Union
	{
		const char*			mName;
		PsArray<UnionType>	mTypes;
	};
} }

#endif
