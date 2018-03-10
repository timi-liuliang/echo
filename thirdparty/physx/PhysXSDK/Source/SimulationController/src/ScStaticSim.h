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


#ifndef PX_PHYSICS_SCP_STATIC_SIM
#define PX_PHYSICS_SCP_STATIC_SIM

#include "ScRigidSim.h"
#include "ScStaticCore.h"

namespace physx
{
namespace Sc
{
	class StaticSim : public RigidSim
	{
		//---------------------------------------------------------------------------------
		// Construction, destruction & initialization
		//---------------------------------------------------------------------------------
	public:
		StaticSim(Scene&, StaticCore&);
		~StaticSim();

		void postActor2WorldChange();

		PX_FORCE_INLINE	StaticCore&	getStaticCore()	const { return static_cast<StaticCore&>(getRigidCore());	}
	};

} // namespace Sc

}

#endif
