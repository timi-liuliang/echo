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


#ifndef PX_PHYSICS_NP_PHYSICS_INSERTION_CALLBACK
#define PX_PHYSICS_NP_PHYSICS_INSERTION_CALLBACK

#include "common/PxPhysicsInsertionCallback.h"
#include "GuTriangleMesh.h"
#include "GuHeightField.h"
#include "NpFactory.h"

namespace physx
{
	class NpPhysicsInsertionCallback: public PxPhysicsInsertionCallback
	{
	public:
		NpPhysicsInsertionCallback() {}

		virtual bool insertObject(PxBase& insertable)
		{			
			PxTriangleMesh* triMesh = insertable.is<PxTriangleMesh> ();			 
			if(triMesh)
			{
				Gu::TriangleMesh* guMesh = static_cast<Gu::TriangleMesh*> (triMesh);
				guMesh->setMeshFactory(&NpFactory::getInstance());
				guMesh->setupMeshInterface();
				NpFactory::getInstance().addTriangleMesh(guMesh);
				return true;
			}

			PxHeightField* hf = insertable.is<PxHeightField> ();
			if(hf)
			{
				Gu::HeightField* guHf = static_cast<Gu::HeightField*> (hf);
				guHf->setMeshFactory(&NpFactory::getInstance());
				NpFactory::getInstance().addHeightField(guHf);
				return true;
			}

			Ps::getFoundation().error(PxErrorCode::eINTERNAL_ERROR, __FILE__, __LINE__, "Inserting object failed: "
				"Object type not supported for insert operation.");

			return false;
		}
	};

}

#endif
