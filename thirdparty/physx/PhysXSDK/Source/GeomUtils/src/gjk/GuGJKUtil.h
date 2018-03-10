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

#ifndef GU_GJKUTIL_H
#define GU_GJKUTIL_H

/*
	This file is used to avoid the inner loop cross DLL calls
*/
namespace physx
{
namespace Gu
{

#define PxGJKStatus PxU32
#define GJK_NON_INTERSECT	0 
#define GJK_CONTACT			1
#define GJK_UNDEFINED		2
#define	GJK_DEGENERATE		3
#define EPA_FAIL			4
#define	EPA_DEGENERATE		5
#define	EPA_CONTACT			6

//#define GJK_RELATIVE_EPSILON		0.0025f//square of 5%.
#define GJK_RELATIVE_EPSILON		0.0004f//square of 2%.

}

}

#endif
