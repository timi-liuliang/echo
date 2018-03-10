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


#ifndef PX_PHYSICS_SCP_SHAPECORE
#define PX_PHYSICS_SCP_SHAPECORE

#include "PsUserAllocated.h"
#include "GuGeometryUnion.h"
#include "PxvGeometry.h"
#include "PsUtilities.h"
#include "PxFiltering.h"
#include "PxShape.h"

namespace physx
{
class PxShape;

namespace Sc
{
	class Scene;
	class RigidCore;
	class BodyCore;
	class ShapeSim;
	class MaterialCore;
	class InteractionScene;

	class ShapeCore : public Ps::UserAllocated
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
// PX_SERIALIZATION
													ShapeCore(const PxEMPTY&);
						void						exportExtraData(PxSerializationContext& stream);
						void						importExtraData(PxDeserializationContext& context);
						void						resolveReferences(PxDeserializationContext& context);
		static			void						getBinaryMetaData(PxOutputStream& stream);
		                void                        resolveMaterialReference(PxU32 materialTableIndex, PxU16 materialIndex);
//~PX_SERIALIZATION

													ShapeCore(const PxGeometry& geometry, 
															  PxShapeFlags shapeFlags,
															  const PxU16* materialIndices, 
															  PxU16 materialCount);

													~ShapeCore();

		PX_FORCE_INLINE	PxGeometryType::Enum		getGeometryType()			const	{ return mCore.geometry.getType(); }
						PxShape*					getPxShape();
						const PxShape*				getPxShape() const;

		PX_FORCE_INLINE	const Gu::GeometryUnion&	getGeometryUnion()			const	{ return mCore.geometry;		}
		PX_FORCE_INLINE	const PxGeometry&			getGeometry()				const	{ return mCore.geometry.getGeometry(); }
						void						setGeometry(const PxGeometry& geom);

						PxU16						getNbMaterialIndices()		const;
						const PxU16*				getMaterialIndices()		const;
						void						setMaterialIndices(const PxU16* materialIndices, PxU16 materialIndexCount);

		PX_FORCE_INLINE	const PxTransform&			getShape2Actor()				const	{ return mCore.transform; }
						void						setShape2Actor(const PxTransform& s2b);

		PX_FORCE_INLINE	const PxFilterData&			getSimulationFilterData()	const	{ return mSimulationFilterData; }
						void						setSimulationFilterData(const PxFilterData& data);

		// PT: this one doesn't need double buffering
		PX_FORCE_INLINE	const PxFilterData&			getQueryFilterData()		const	{ return mQueryFilterData; }
		PX_FORCE_INLINE	void						setQueryFilterData(const PxFilterData& data)	{ mQueryFilterData = data; }

		PX_FORCE_INLINE	PxReal						getContactOffset()			const	{ return mCore.contactOffset;	}
		PX_FORCE_INLINE	void						setContactOffset(PxReal s)			{ mCore.contactOffset = s;		}

		PX_FORCE_INLINE	PxReal						getRestOffset()				const	{ return mRestOffset; }
						void						setRestOffset(PxReal);

		PX_FORCE_INLINE	PxShapeFlags				getFlags()					const	{ return PxShapeFlags(mCore.mShapeFlags);	}
						void						setFlags(PxShapeFlags f);

		PX_FORCE_INLINE const PxsShapeCore&			getCore()					const	{ return mCore; }

		static PX_FORCE_INLINE ShapeCore&			getCore(PxsShapeCore& core)			
		{ 
			size_t offset = offsetof(ShapeCore, mCore);
			return *reinterpret_cast<ShapeCore*>(reinterpret_cast<PxU8*>(&core) - offset); 
		}	

	protected:
						PxFilterData				mQueryFilterData;		// Query filter data PT: TODO: consider moving this to SceneQueryShapeData
						PxFilterData				mSimulationFilterData;	// Simulation filter data
						PxsShapeCore				PX_ALIGN(16, mCore);	
						PxReal						mRestOffset;			// same as the API property of the same name
						bool						mOwnsMaterialIdxMemory;	// For de-serialization to avoid deallocating material index list
	};

} // namespace Sc


}

#endif
