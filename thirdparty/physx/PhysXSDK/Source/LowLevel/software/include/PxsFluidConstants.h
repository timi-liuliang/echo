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


#ifndef PXS_FLUID_CONSTANTS_H
#define PXS_FLUID_CONSTANTS_H

// Threshold for the angle between two contact constraint surfaces. If the two surfaces form
// a "steep valley" only one of the two constraints will be applied.
#define PXS_FLUID_COLL_VEL_PROJECTION_CROSS_EPSILON		1e-6f

#define PXS_FLUID_COLL_VEL_PROJECTION_PROJ				1e-4f
#define PXS_FLUID_COLL_TRI_DISTANCE						1e-5f
#define PXS_FLUID_COLL_RAY_EPSILON_FACTOR				1e-4f

#endif // PXS_FLUID_CONSTANTS_H
