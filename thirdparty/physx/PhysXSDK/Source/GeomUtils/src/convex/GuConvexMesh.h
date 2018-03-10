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

#ifndef GU_COLLISION_CONVEXMESH_H
#define GU_COLLISION_CONVEXMESH_H

#include "PxConvexMesh.h"
#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "CmRefCountable.h"
#include "PxBitAndData.h"
#include "GuConvexMeshData.h"

// PX_SERIALIZATION
#include "PxMetaData.h"
#include "CmRenderOutput.h"
//~PX_SERIALIZATION

namespace physx
{

class BigConvexData;
class GuMeshFactory;
class PxInputStream;

namespace Cm
{
	class Matrix34;
}



namespace Gu
{
	struct HullPolygonData;

	PX_INLINE PxU32 computeBufferSize(const Gu::ConvexHullData& data, PxU32 nb)
	{
		PxU32 bytesNeeded = sizeof(Gu::HullPolygonData) * data.mNbPolygons;
		bytesNeeded += sizeof(PxVec3) * data.mNbHullVertices;
		bytesNeeded += sizeof(PxU8) * data.mNbEdges * 2;		// mFacesByEdges8
		bytesNeeded += sizeof(PxU8) * data.mNbHullVertices * 3; // mFacesByVertices8;
		bytesNeeded += sizeof(PxU8) * nb;						// mVertexData8
		

		//4 align the whole thing!
		const PxU32 mod = bytesNeeded % sizeof(PxReal);
		if (mod)
			bytesNeeded += sizeof(PxReal) - mod;
		return bytesNeeded;
	}

	// 0: includes raycast map
	// 1: discarded raycast map
	// 2: support map not always there
	// 3: support stackless trees for non-recursive collision queries
	// 4: no more opcode model
	// 5: valencies table and gauss map combined, only exported over a vertex count treshold that depends on the platform cooked for.
	// 6: removed support for edgeData16.
	// 7: removed support for edge8Data.
	// 8: removed support for triangles.

	// 9: removed local sphere.
	//10: removed geometric center.
	//11: removed mFlags, and mERef16 from Poly; nbVerts is just a byte.
	//12: removed explicit minimum, maximum from Poly
	//13: internal objects
    #define  PX_CONVEX_VERSION 13
  
	class ConvexMesh : public PxConvexMesh, public Ps::UserAllocated, public Cm::RefCountable
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
	public:
	// PX_SERIALIZATION
		PX_PHYSX_COMMON_API 						ConvexMesh(PxBaseFlags baseFlags) : PxConvexMesh(baseFlags), Cm::RefCountable(PxEmpty), mNb(PxEmpty)
													{
														mNb.setBit();
													}

		PX_PHYSX_COMMON_API virtual	void			exportExtraData(PxSerializationContext& stream);
		PX_PHYSX_COMMON_API			void			importExtraData(PxDeserializationContext& context);
		PX_PHYSX_COMMON_API virtual	void			onRefCountZero();
		PX_PHYSX_COMMON_API static	ConvexMesh*		createObject(PxU8*& address, PxDeserializationContext& context);
		PX_PHYSX_COMMON_API static	void			getBinaryMetaData(PxOutputStream& stream);
									void			resolveReferences(PxDeserializationContext&)				{}
							virtual	void			requires(PxProcessPxBaseCallback&){}
	//~PX_SERIALIZATION
		PX_PHYSX_COMMON_API 						ConvexMesh();

		PX_PHYSX_COMMON_API bool					load(PxInputStream& stream);

		// PxConvexMesh										
		PX_PHYSX_COMMON_API virtual	void			release();
		PX_PHYSX_COMMON_API virtual	PxU32			getNbVertices()									const	{ return mHullData.mNbHullVertices;		}
		PX_PHYSX_COMMON_API virtual	const PxVec3*	getVertices()									const	{ return mHullData.getHullVertices();	}
		PX_PHYSX_COMMON_API virtual	const PxU8*		getIndexBuffer()								const	{ return mHullData.getVertexData8();	}
		PX_PHYSX_COMMON_API virtual	PxU32			getNbPolygons()									const	{ return mHullData.mNbPolygons;			}
		PX_PHYSX_COMMON_API virtual	bool			getPolygonData(PxU32 i, PxHullPolygon& data)	const;
		PX_PHYSX_COMMON_API virtual	PxU32			getReferenceCount()								const;
		PX_PHYSX_COMMON_API virtual	void			getMassInformation(PxReal& mass, PxMat33& localInertia, PxVec3& localCenterOfMass)	const;
		PX_PHYSX_COMMON_API virtual	PxBounds3		getLocalBounds()								const	{ PX_ASSERT(mHullData.mAABB.isValid()); return mHullData.mAABB; }
		//~PxConvexMesh

		PX_FORCE_INLINE	PxU32					getNbVerts()										const	{ return mHullData.mNbHullVertices;		}
		PX_FORCE_INLINE	const PxVec3*			getVerts()											const	{ return mHullData.getHullVertices();	}
		PX_FORCE_INLINE	PxU32					getNbPolygonsFast()									const	{ return mHullData.mNbPolygons;			}
		PX_FORCE_INLINE	const HullPolygonData&	getPolygon(PxU32 i)									const	{ return mHullData.mPolygons[i];		}
		PX_FORCE_INLINE	const HullPolygonData*	getPolygons()										const	{ return mHullData.mPolygons;			}
		PX_FORCE_INLINE	PxU32					getNbEdges()										const	{ return mHullData.mNbEdges;			}

		PX_FORCE_INLINE	const ConvexHullData&	getHull()											const	{ return mHullData;						}
		PX_FORCE_INLINE	ConvexHullData&			getHull()													{ return mHullData;						}
		PX_FORCE_INLINE	const PxBounds3&		getLocalBoundsFast()								const	{ return mHullData.mAABB;				}
		PX_FORCE_INLINE	PxReal					getMass()											const	{ return mMass;							}
		PX_FORCE_INLINE	const PxMat33&			getInertia()										const	{ return mInertia;						}

		PX_FORCE_INLINE BigConvexData*			getBigConvexData()									const	{ return mBigConvexData;				}
		PX_FORCE_INLINE void					setBigConvexData(BigConvexData* bcd)						{ mBigConvexData = bcd;					}

		PX_FORCE_INLINE	PxU32					getBufferSize()										const	{ return computeBufferSize(mHullData, getNb());	}

		PX_PHYSX_COMMON_API virtual				~ConvexMesh();

		PX_FORCE_INLINE	void					setMeshFactory(GuMeshFactory* f)							{ mMeshFactory = f;						}

	protected:
						ConvexHullData			mHullData;
						PxBitAndDword			mNb;	// ### PT: added for serialization. Try to remove later?

						BigConvexData*			mBigConvexData;		//!< optional, only for large meshes! PT: redundant with ptr in chull data? Could also be end of other buffer
						PxReal					mMass;				//this is mass assuming a unit density that can be scaled by instances!
						PxMat33					mInertia;			//in local space of mesh!
private:
						GuMeshFactory*			mMeshFactory;	// PT: changed to pointer for serialization

		PX_FORCE_INLINE	PxU32					getNb()												const	{ return mNb;						}
		PX_FORCE_INLINE	PxU32					ownsMemory()										const	{ return PxU32(!mNb.isBitSet());			}

#if PX_ENABLE_DEBUG_VISUALIZATION
public:
	/**
	\brief Perform convex mesh geometry debug visualization

	\param[out] Debug renderer.
	\param[out] World position.
	*/
						void					debugVisualize(	Cm::RenderOutput& out, const Cm::Matrix34& absPose, const PxBounds3& cullbox,
																const PxU64 mask, const PxReal fscale, const PxU32 numMaterials)	const;

#endif
	};

} // namespace Gu

}

#endif
