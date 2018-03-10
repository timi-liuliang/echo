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

#include "PxPreprocessor.h"
#include "PxContactModifyCallback.h"
#include "PsVecMath.h"
#include "PsUtilities.h"

#ifdef PX_SUPPORT_SIMD

#include "PsMathUtils.h"
#include "PxcSolverContact.h"
#include "PxcSolverContactPF.h"
#include "PxcSolverConstraintTypes.h"
#include "PxcNpWorkUnit.h"

#include "PxcNpThreadContext.h"
#include "PxcCorrelationBuffer.h"

#include "PsVecMath.h"

#include "PxvDynamics.h"
#include "PxsMaterialManager.h"
#include "PxsMaterialCombiner.h"

using namespace physx;
using namespace Gu;
using namespace Ps::aos;

#endif
