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


#include "ScbShape.h"

using namespace physx;

bool Scb::Shape::setMaterialsHelper(PxMaterial* const* materials, PxU16 materialCount)
{
	PX_ASSERT(!isBuffering());

	if (materialCount == 1)
	{
		PxU16 materialIndex = Ps::to16((static_cast<NpMaterial*>(materials[0]))->getHandle());

		mShape.setMaterialIndices(&materialIndex, 1);
	}
	else
	{
		PX_ASSERT(materialCount > 1);

		PX_ALLOCA(materialIndices, PxU16, materialCount);

		if (materialIndices)
		{
			NpMaterial::getMaterialIndices(materials, materialIndices, materialCount);
			mShape.setMaterialIndices(materialIndices, materialCount);
		}
		else
		{
			Ps::getFoundation().error(PxErrorCode::eOUT_OF_MEMORY, __FILE__, __LINE__, 
				"Shape::setMaterials() failed. Out of memory. Call will be ignored.");
			return false;
		}
	}

	return true;
}


void Scb::Shape::syncState()
{
	PxU32 flags = getBufferFlags();
	if (flags)
	{

		PxShapeFlags oldShapeFlags = mShape.getFlags();

		const Scb::ShapeBuffer&	buffer = *getBufferedData();

		if (flags & Buf::BF_Geometry)
		{
			mShape.setGeometry(buffer.geometry.getGeometry());

#if PX_SUPPORT_VISUAL_DEBUGGER
			if(getControlState() == ControlState::eIN_SCENE)
			{
				Scb::Scene* scbScene = getScbScene();
				PX_ASSERT(scbScene);
				if (scbScene->getSceneVisualDebugger().isConnected(true))
					scbScene->getSceneVisualDebugger().releaseAndRecreateGeometry(this);
			}
#endif
		}

		if (flags & Buf::BF_Material)
		{
			const PxU16* materialIndices = getMaterialBuffer(*getScbScene(), buffer);
			mShape.setMaterialIndices(materialIndices, buffer.materialCount);
			UPDATE_PVD_MATERIALS()
			// TODO: So far we did not bother to fail gracefully in the case of running out of memory. If that should change then this
			// method is somewhat problematic. The material ref counters have been adjusted at the time when the public API was called.
			// Could be that one of the old materials was deleted afterwards. The problem now is what to do if this method fails?
			// We can't adjust the material ref counts any longer since some of the old materials might have been deleted.
			// One solution could be that this class allocates an array of material pointers when the buffered method is called.
			// This array is then passed into the core object and is used by the core object, i.e., the core object does not allocate the
			// buffer itself.
		}

		flush<Buf::BF_Shape2Actor>(buffer);
		flush<Buf::BF_SimulationFilterData>(buffer);
		flush<Buf::BF_ContactOffset>(buffer);
		flush<Buf::BF_RestOffset>(buffer);
		flush<Buf::BF_Flags>(buffer);

		Sc::RigidCore* scRigidCore = NpShapeGetScRigidObjectFromScbSLOW(*this);

		if (scRigidCore) // may be NULL for exclusive shapes because of pending shape updates after buffered release of actor.
		{
			scRigidCore->onShapeChange(mShape, Sc::ShapeChangeNotifyFlags(flags), oldShapeFlags);
		}
	}

	postSyncState();
}
