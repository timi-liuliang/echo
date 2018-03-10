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

#ifndef GU_ENTITY_REPORT_H
#define GU_ENTITY_REPORT_H

#include "Ps.h"
#include "PxQueryReport.h"

namespace physx
{
namespace Gu
{

template<class T>
class EntityReport
{
	public:
	virtual			~EntityReport()	{}
	virtual	PxAgain	onEvent(PxU32 nbEntities, T* entities) = 0;
};


}  // namespace Gu

}

#endif
