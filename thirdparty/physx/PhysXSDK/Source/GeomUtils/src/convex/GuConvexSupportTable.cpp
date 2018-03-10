/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "GuVecBox.h"

namespace physx
{
	const Ps::aos::BoolV boxVertexTable[8] = {
										Ps::aos::BFFFF(),//---
										Ps::aos::BTFFF(),//+--
										Ps::aos::BFTFF(),//-+-
										Ps::aos::BTTFF(),//++-
										Ps::aos::BFFTF(),//--+
										Ps::aos::BTFTF(),//+-+
										Ps::aos::BFTTF(),//-++
										Ps::aos::BTTTF(),//+++
									}; 
}
