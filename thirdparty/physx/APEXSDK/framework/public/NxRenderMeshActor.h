/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_RENDER_MESH_ACTOR_H
#define NX_RENDER_MESH_ACTOR_H

/*!
\file
\brief class NxRenderMeshActor
*/

#include "NxApexActor.h"
#include "NxApexRenderable.h"
#include "NxUserRenderResourceManager.h"	// For NxRenderCullMode
#include "foundation/PxVec3.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

class NxUserRenderInstanceBuffer;

/**
\brief Flags used for raycasting an NxRenderMeshActor
*/
struct NxRenderMeshActorRaycastFlags
{
	/**
	\brief Enum of flags used for raycasting an NxRenderMeshActor
	*/
	enum Enum
	{
		VISIBLE_PARTS =	(1 << 0),
		INVISIBLE_PARTS =	(1 << 1),

		ALL_PARTS =			VISIBLE_PARTS | INVISIBLE_PARTS
	};
};

/**
\brief Return data from raycasting an NxRenderMeshActor
*/
struct NxRenderMeshActorRaycastHitData
{
	/**
		The time to the hit point.  That is, the hit point is given by worldOrig + time*worldDisp.
	*/
	physx::PxF32	time;

	/**
		The part index containing the hit triangle.
	*/
	physx::PxU32	partIndex;

	/**
		The submesh index of the triangle hit.
	*/
	physx::PxU32	submeshIndex;

	/**
		The vertex indices of the triangle hit.
	*/
	physx::PxU32	vertexIndices[3];

	/**
		The lighting normal at the hit point, if the vertex format supports this.  Otherwise set to (0,0,0).
	*/
	physx::PxVec3	normal;

	/**
		The lighting tangent at the hit point, if the vertex format supports this.  Otherwise set to (0,0,0).
	*/
	physx::PxVec3	tangent;

	/**
		The lighting binormal at the hit point, if the vertex format supports this.  Otherwise set to (0,0,0).
	*/
	physx::PxVec3	binormal;
};


/**
\brief Renderable mesh (dynamic data associated with NxRenderMeshAsset)
*/
class NxRenderMeshActor : public NxApexActor, public NxApexRenderable
{
public:

	enum
	{
		InvalidInstanceID = 0xFFFFFFFF
	};

	/**
	\brief Returns the visibilities of all mesh parts in the given array.

	The user must supply the array size.  If the size is less than the
	part count, then the list will be truncated to fit the given array.

	Returns true if any of the visibility values in visibilityArray are changed.
	*/
	virtual bool					getVisibilities(physx::PxU8* visibilityArray, physx::PxU32 visibilityArraySize) const = 0;

	/**
	\brief Set the visibility of the indexed part.  Returns true iff the visibility for the part is changed by this operation.
	*/
	virtual bool					setVisibility(bool visible, physx::PxU16 partIndex = 0) = 0;

	/**
	\brief Returns the visibility of the indexed part.
	*/
	virtual bool					isVisible(physx::PxU16 partIndex = 0) const = 0;

	/**
	\brief Returns the number of visible parts.
	*/
	virtual physx::PxU32			visiblePartCount() const = 0;

	/**
	\brief Returns an array of visible part indices.

	The size of this array is given by visiblePartCount().  Note: the
	indices are in an arbitrary order.
	*/
	virtual const physx::PxU32*		getVisibleParts() const = 0;

	/**
	\brief Returns the number of bones used by this render mesh
	*/
	virtual physx::PxU32            getBoneCount() const = 0;

	/**
	\brief Sets the local-to-world transform for the indexed bone.  The transform need not be orthonormal.
	*/
	virtual void					setTM(const physx::PxMat44& tm, physx::PxU32 boneIndex = 0) = 0;

	/**
	\brief Same as setTM(), but assumes tm is pure rotation.

	This can allow some optimization.  The user must supply scaling
	separately.  The scale vector is interpreted as the diagonal of a
	diagonal matrix, applied before the rotation component of tm.
	*/
	virtual void					setTM(const physx::PxMat44& tm, const physx::PxVec3& scale, physx::PxU32 boneIndex = 0) = 0;

	/**
	\brief Update the axis-aligned bounding box which encloses all visible parts in their world-transformed poses.
	*/
	virtual void					updateBounds() = 0;

	/**
	\brief Returns the local-to-world transform for the indexed bone.
	*/
	virtual const physx::PxMat44	getTM(physx::PxU32 boneIndex = 0) const = 0;

	/**
	\brief If the number of visible parts becomes 0, or if instancing and the number of instances
	becomes 0, then release resources if this bool is true.
	*/
	virtual void					setReleaseResourcesIfNothingToRender(bool value) = 0;

	/**
	\brief If this set to true, render visibility will not be updated until the user calls syncVisibility().
	*/
	virtual void					setBufferVisibility(bool bufferVisibility) = 0;

	/**
	\brief Sets the override material for the submesh with the given index.
	*/
	virtual void					setOverrideMaterial(PxU32 submeshIndex, const char* overrideMaterialName) = 0;

	/**
	\brief Sync render visibility with that set by the user.  Only
	needed if bufferVisibility(true) is called, or bufferVisibility = true
	in the actor's descriptor.

	If useLock == true, the RenderMeshActor's lock is used during the sync.
	*/
	virtual void					syncVisibility(bool useLock = true) = 0;

	/**
	\brief get the user-provided instance buffer.
	*/
	virtual NxUserRenderInstanceBuffer* getInstanceBuffer() const = 0;
	/**
	\brief applies the user-provided instance buffer to all submeshes.
	*/
	virtual void					setInstanceBuffer(NxUserRenderInstanceBuffer* instBuf) = 0;
	/**
	\brief allows the user to change the max instance count in the case that the instance buffer was changed
	*/
	virtual void					setMaxInstanceCount(physx::PxU32 count) = 0;
	/**
	\brief sets the range for the instance buffer

	\param from the position in the buffer (measured in number of elements) to start reading from
	\param count number of instances to be rendered. Must not exceed maxInstances for this actor
	*/
	virtual void					setInstanceBufferRange(physx::PxU32 from, physx::PxU32 count) = 0;

	/**
		Returns true if and only if a part is hit matching various criteria given in the function
		parameters. If a part is hit, the hitData field contains information about the ray intersection.
		(hitData may be modified even if the function returns false.)
			hitData (output) = information about the mesh at the hitpoint.  See NxRenderMeshActorRaycastHitData.
			worldOrig = the origin of the ray in world space
			worldDisp = the displacement of the ray in world space (need not be normalized)
			flags = raycast control flags (see NxRenderMeshActorRaycastFlags)
			winding = winding filter for hit triangle.  If NxRenderCullMode::CLOCKWISE or
				NxRenderCullMode::COUNTER_CLOCKWISE, then triangles will be assumed to have
				that winding, and will only contribute to the raycast if front-facing.  If
				NxRenderCullMode::NONE, then all triangles will contribute.
			partIndex = If -1, then all mesh parts will be raycast, and the result returned for
				the earliest hit.  Otherwise only the part indexed by partIndex will be raycast.

		N.B. Currently only works for static (unskinned) and one transform per-part, single-weighted vertex skinning.
	*/
	virtual bool					rayCast(NxRenderMeshActorRaycastHitData& hitData,
	                                        const physx::PxVec3& worldOrig, const physx::PxVec3& worldDisp,
	                                        NxRenderMeshActorRaycastFlags::Enum flags = NxRenderMeshActorRaycastFlags::VISIBLE_PARTS,
	                                        NxRenderCullMode::Enum winding = NxRenderCullMode::CLOCKWISE,
	                                        physx::PxI32 partIndex = -1) const = 0;

protected:

	virtual							~NxRenderMeshActor() {}
};

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_RENDER_MESH_ACTOR_H
