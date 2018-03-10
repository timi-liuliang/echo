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


#ifndef PX_COLLISION_TRIANGLEMESHBUILDER
#define PX_COLLISION_TRIANGLEMESHBUILDER

#include "GuTriangleMesh.h"
#include "cooking/PxCooking.h"

namespace physx
{

class PxTriangleMeshDesc;

	class TriangleMeshBuilder : public Gu::TriangleMesh
	{
	public:
				bool					loadFromDesc(const PxTriangleMeshDesc&, const PxCookingParams& params,bool validate = false);
				bool					save(PxOutputStream& stream, bool platformMismatch, const PxCookingParams& params) const;
	protected:
				void					computeLocalBounds();
				bool					importMesh(const PxTriangleMeshDesc& desc, const PxCookingParams& params,bool validate = false);

//	static		void					SaveObj(const PxTriangleMeshDesc &source,const char *fname); // debug tool
	};

}

#endif
