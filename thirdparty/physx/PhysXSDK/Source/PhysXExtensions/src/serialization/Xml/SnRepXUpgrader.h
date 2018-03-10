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
#ifndef PX_REPX_UPGRADER_H
#define PX_REPX_UPGRADER_H

#include "PxSimpleTypes.h"

namespace physx { namespace Sn {
	class RepXCollection;

	class RepXUpgrader
	{
	public:
		//If a new collection is created, the source collection is destroyed.
		//Thus you only need to release the new collection.
		//This holds for all of the upgrade functions.
		//So be aware, that the argument to these functions may not be valid
		//after they are called, but the return value always will be valid.
		static RepXCollection& upgradeCollection( RepXCollection& src );
		static RepXCollection& upgrade10CollectionTo3_1Collection( RepXCollection& src );
		static RepXCollection& upgrade3_1CollectionTo3_2Collection( RepXCollection& src );
		static RepXCollection& upgrade3_2CollectionTo3_3Collection( RepXCollection& src );
	};
} }

#endif
