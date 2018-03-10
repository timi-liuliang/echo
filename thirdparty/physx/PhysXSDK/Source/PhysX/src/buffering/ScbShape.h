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


#ifndef PX_PHYSICS_SCB_SHAPE
#define PX_PHYSICS_SCB_SHAPE

#include "NpMaterial.h"
#include "NpPhysics.h"
#include "ScbNpDeps.h"
#include "ScShapeCore.h"
#include "ScRigidCore.h"

#include "PsUtilities.h"

// PX_SERIALIZATION
#include "PxSerialFramework.h"
//~PX_SERIALIZATION

#include "ScbDefs.h"

namespace physx
{


#if PX_SUPPORT_VISUAL_DEBUGGER
	#define UPDATE_PVD_MATERIALS()												\
	if(getControlState() == ControlState::eIN_SCENE)							\
	{																			\
		Scb::Scene* scene = getScbScene();										\
		PX_ASSERT(scene);														\
		if(scene->getSceneVisualDebugger().isConnected(true))						\
			scene->getSceneVisualDebugger().updateMaterials(this);				\
	}
#else
	#define UPDATE_PVD_MATERIALS() {}
#endif

namespace Scb
{

class RigidObject;

struct ShapeBuffer
{
	template <PxU32 I, PxU32 dummy> struct Fns {};  // TODO: make the base class traits visible
	typedef Sc::ShapeCore Core;
	typedef ShapeBuffer Buf;

	ShapeBuffer() : materialBufferIndex(0), materialCount(0) {}

	SCB_REGULAR_ATTRIBUTE_ALIGNED(2, PxTransform,	Shape2Actor, 16)
	SCB_REGULAR_ATTRIBUTE(3, PxFilterData,  SimulationFilterData)
	SCB_REGULAR_ATTRIBUTE(4, PxReal,		ContactOffset)
	SCB_REGULAR_ATTRIBUTE(5, PxReal,		RestOffset)
	SCB_REGULAR_ATTRIBUTE(6, PxShapeFlags,	Flags)

	Gu::GeometryUnion	geometry;

	union
	{
		PxU16			materialIndex;  // for single material shapes
		PxU32			materialBufferIndex;  // for multi material shapes
	};
	PxU16				materialCount;

	enum 
	{
		BF_Geometry			= 1<<0,
		BF_Material			= 1<<1
	};

};

class Shape : public Base
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================

	typedef Sc::ShapeCore Core;
	typedef ShapeBuffer Buf;
public:
// PX_SERIALIZATION
										Shape(const PxEMPTY&) :	Base(PxEmpty), mShape(PxEmpty)	{}
	static		void					getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	PX_INLINE							Shape(const PxGeometry& geometry,
											  PxShapeFlags shapeFlags,
											  const PxU16* materialIndices,
											  PxU16 materialCount,
											  bool isExclusive);

	PX_INLINE	PxGeometryType::Enum	getGeometryType() const;

	PX_INLINE	const PxGeometry&		getGeometry() const;
	PX_INLINE	const Gu::GeometryUnion&getGeometryUnion() const;
	PX_INLINE	Scb::ShapeBuffer*		setGeometry(const PxGeometry& geom);

	PX_INLINE	PxU16					getNbMaterials() const;
	PX_INLINE	PxMaterial*				getMaterial(PxU32 index) const;
	PX_INLINE	PxU32					getMaterials(PxMaterial** buffer, PxU32 bufferSize) const;
	PX_INLINE	bool					setMaterials(PxMaterial*const* materials, PxU16 materialCount);

	PX_INLINE	const PxTransform&		getShape2Actor() const							{ return read<Buf::BF_Shape2Actor>();			}
	PX_INLINE	void					setShape2Actor(const PxTransform& v)				{ write<Buf::BF_Shape2Actor>(v);					}
	
	PX_INLINE	PxFilterData			getSimulationFilterData() const					{ return read<Buf::BF_SimulationFilterData>();	}
	PX_INLINE	void					setSimulationFilterData(const PxFilterData& v)	{ write<Buf::BF_SimulationFilterData>(v);		}

	PX_INLINE	PxReal					getContactOffset() const						{ return read<Buf::BF_ContactOffset>();			}
	PX_INLINE	void					setContactOffset(PxReal v)						{ write<Buf::BF_ContactOffset>(v);				}

	PX_INLINE	PxReal					getRestOffset() const							{ return read<Buf::BF_RestOffset>();			}
	PX_INLINE	void					setRestOffset(PxReal v)							{ write<Buf::BF_RestOffset>(v);					}

	PX_INLINE	PxShapeFlags			getFlags() const								{ return read<Buf::BF_Flags>();					}
	PX_INLINE	void					setFlags(PxShapeFlags v)						{ write<Buf::BF_Flags>(v);						}


	//---------------------------------------------------------------------------------
	// Data synchronization
	//---------------------------------------------------------------------------------
				void					syncState();


	//---------------------------------------------------------------------------------
	// Miscellaneous
	//---------------------------------------------------------------------------------
	PX_FORCE_INLINE const PxU16*		getScMaterialIndices()				const	{ return mShape.getMaterialIndices();	}  // Only use if you know what you're doing!

	PX_FORCE_INLINE Sc::ShapeCore&			getScShape()							{ return mShape;					}  // Only use if you know what you're doing!
	PX_FORCE_INLINE const Sc::ShapeCore&	getScShape()					const	{ return mShape;					}

	PX_FORCE_INLINE bool				isExclusive()						const	{ return getScbType() == ScbType::SHAPE_EXCLUSIVE; }
	PX_FORCE_INLINE void				setControlStateIfExclusive(Scene* s, ControlState::Enum cs);  // for exclusive shapes

	template<bool sync> PX_FORCE_INLINE void checkUpdateOnRemove(Scene* s);

	static size_t getScOffset()													
	{ 
		return reinterpret_cast<size_t>(&reinterpret_cast<Shape*>(0)->mShape);
	}

private:
	bool									setMaterialsHelper(PxMaterial* const* materials, PxU16 materialCount);

	Sc::ShapeCore			mShape;

	PX_FORCE_INLINE	const Scb::ShapeBuffer*	getBufferedData()	const	{ return (const Scb::ShapeBuffer*)getStream();	}
	PX_FORCE_INLINE	Scb::ShapeBuffer*		getBufferedData()			{ return (Scb::ShapeBuffer*)getStream();		}


	PX_FORCE_INLINE	const PxU16* getMaterialBuffer(const Scb::Scene& scene, const Scb::ShapeBuffer& sb) const
	{
		if (sb.materialCount == 1)
			return &sb.materialIndex;
		else
			return scene.getShapeMaterialBuffer(sb.materialBufferIndex);
	}

	//---------------------------------------------------------------------------------
	// Infrastructure for regular attributes
	//---------------------------------------------------------------------------------

	struct Access: public BufferedAccess<Buf, Core, Shape> 
	{
		template<typename Fns>
		static PX_FORCE_INLINE void write(Shape& base, Core& core, typename Fns::Arg v)
		{
			if (!base.isBuffering())
			{
				PxShapeFlags oldShapeFlags = core.getFlags();
				Fns::setCore(core, v);

				// shared shapes return NULL. But shared shapes aren't mutable when attached to an actor, so no notification needed.
				Sc::RigidCore* rigidCore = NpShapeGetScRigidObjectFromScbSLOW(base);
				if(rigidCore && base.getControlState() != ControlState::eINSERT_PENDING)
					rigidCore->onShapeChange(core, Sc::ShapeChangeNotifyFlags(Fns::flag), oldShapeFlags);
#if PX_SUPPORT_VISUAL_DEBUGGER
				Scb::Scene* scene = base.getScbSceneForAPI();	// shared shapes also return zero here
				if(scene && !base.insertPending() && scene->getSceneVisualDebugger().isConnected(true))
					 scene->getSceneVisualDebugger().updatePvdProperties(&base);
#endif
			}
			else if (base.fetchDataBuffer())
			{
				Fns::setBuffered(*reinterpret_cast<Buf*>(base.getStream()), v);
				base.markUpdated(Fns::flag);
			}
	}

	};
	template<PxU32 f> PX_FORCE_INLINE typename Buf::Fns<f,0>::Arg read() const		{	return Access::read<Buf::Fns<f,0> >(*this, mShape);	}
	template<PxU32 f> PX_FORCE_INLINE void write(typename Buf::Fns<f,0>::Arg v)		{	Access::write<Buf::Fns<f,0> >(*this, mShape, v);	}
	template<PxU32 f> PX_FORCE_INLINE void flush(const Buf& buf)					{	Access::flush<Buf::Fns<f,0> >(*this, mShape, buf);	}

};


PX_INLINE Shape::Shape(const PxGeometry& geometry,
					   PxShapeFlags shapeFlags,
					   const PxU16* materialIndices,
					   PxU16 materialCount,
					   bool isExclusive) :
	mShape(geometry, shapeFlags, materialIndices, materialCount)
{
	// paranoia: the notify flags in Sc have to match up

	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_Geometry				== (PxU32)Sc::ShapeChangeNotifyFlag::eGEOMETRY);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_Material				== (PxU32)Sc::ShapeChangeNotifyFlag::eMATERIAL);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_Shape2Actor			== (PxU32)Sc::ShapeChangeNotifyFlag::eSHAPE2BODY);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_SimulationFilterData	== (PxU32)Sc::ShapeChangeNotifyFlag::eFILTERDATA);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_ContactOffset			== (PxU32)Sc::ShapeChangeNotifyFlag::eCONTACTOFFSET);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_RestOffset			== (PxU32)Sc::ShapeChangeNotifyFlag::eRESTOFFSET);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_Flags					== (PxU32)Sc::ShapeChangeNotifyFlag::eFLAGS);
	PX_COMPILE_TIME_ASSERT((PxU32)ShapeBuffer::BF_Geometry				== (PxU32)Sc::ShapeChangeNotifyFlag::eGEOMETRY);

	if (isExclusive)
		setScbType(ScbType::SHAPE_EXCLUSIVE);
	else
		setScbType(ScbType::SHAPE_SHARED);
}


PX_INLINE PxGeometryType::Enum Shape::getGeometryType() const
{
	return mShape.getGeometryType();
}

PX_INLINE const PxGeometry& Shape::getGeometry() const
{
	if (isBuffered(Buf::BF_Geometry))
		return getBufferedData()->geometry.getGeometry();
	else
		return mShape.getGeometry();
}

PX_INLINE const Gu::GeometryUnion& Shape::getGeometryUnion() const
{
	if (isBuffered(Buf::BF_Geometry))
		return getBufferedData()->geometry;
	else
		return mShape.getGeometryUnion();
}


PX_INLINE Scb::ShapeBuffer* Shape::setGeometry(const PxGeometry& geom)
{
	Scb::ShapeBuffer* shapeBuffer = NULL;
	if (!isBuffering())
	{
		mShape.setGeometry(geom);
		Sc::RigidCore* rigidCore = NpShapeGetScRigidObjectFromScbSLOW(*this);
		if(rigidCore)
			rigidCore->onShapeChange(mShape, Sc::ShapeChangeNotifyFlag::eGEOMETRY, PxShapeFlags());

#if PX_SUPPORT_VISUAL_DEBUGGER
		Scb::Scene* scbScene = getScbSceneForAPI();
		if(scbScene && scbScene->getSceneVisualDebugger().isConnected(true))
		{
			scbScene->getSceneVisualDebugger().releaseAndRecreateGeometry( this );
		}
#endif
	}
	else if (fetchDataBuffer())
	{
		markUpdated(Buf::BF_Geometry);
		shapeBuffer = getBufferedData();
		shapeBuffer->geometry.set(geom);
	}

	return shapeBuffer;
}


PX_INLINE PxU16 Shape::getNbMaterials() const
{
	if (isBuffered(Buf::BF_Material))
		return getBufferedData()->materialCount;
	else
		return mShape.getNbMaterialIndices();
}


PX_INLINE PxMaterial* Shape::getMaterial(PxU32 index) const
{
	PX_ASSERT(index < getNbMaterials());

	NpMaterialManager& matManager = NpPhysics::getInstance().getMaterialManager();
	if (isBuffered(Buf::BF_Material))
	{
		const PxU16* materialIndices = getMaterialBuffer(*getScbScene(), *getBufferedData());
		return matManager.getMaterial(materialIndices[index]);
	}
	else
	{
		PxU16 matTableIndex = mShape.getMaterialIndices()[index];
		return matManager.getMaterial(matTableIndex);
	}
}


PX_INLINE PxU32 Shape::getMaterials(PxMaterial** buffer, PxU32 bufferSize) const
{
	const PxU16* materialIndices;
	PxU32 matCount;
	NpMaterialManager& matManager = NpPhysics::getInstance().getMaterialManager();
	if (isBuffered(Buf::BF_Material))
	{
		// IMPORTANT:
		// As long as the material pointers get copied to a user buffer, this works fine.
		// Never give direct access to the internal material buffer because in the
		// double buffered case the pointer changes on resize.

		const Scb::ShapeBuffer*	PX_RESTRICT bufferedData = getBufferedData();

		materialIndices = getMaterialBuffer(*getScbScene(), *bufferedData);
		matCount = bufferedData->materialCount;
	}
	else
	{
		materialIndices = mShape.getMaterialIndices();
		matCount = mShape.getNbMaterialIndices();
	}

	const PxU32 writeCount = PxMin(matCount, bufferSize);
	for(PxU32 i=0; i<writeCount; i++)
		buffer[i] = matManager.getMaterial(materialIndices[i]);

	return writeCount;
}


PX_INLINE bool Shape::setMaterials(PxMaterial* const* materials, PxU16 materialCount)
{
	if (!isBuffering())
	{
		bool ret = setMaterialsHelper(materials, materialCount);
		UPDATE_PVD_MATERIALS()
		return ret;
	}
	else if (fetchDataBuffer())
	{
		Scb::ShapeBuffer* PX_RESTRICT bufferedData = getBufferedData();

		PxU16* materialIndices;
		if (materialCount == 1)
			materialIndices = &bufferedData->materialIndex;
		else
		{
			PxU32 bufferIdx;
			materialIndices = getScbScene()->allocShapeMaterialBuffer(materialCount, bufferIdx);
			bufferedData->materialBufferIndex = bufferIdx;
		}
		bufferedData->materialCount = materialCount;

		NpMaterial::getMaterialIndices(materials, materialIndices, materialCount);
		
		markUpdated(Buf::BF_Material);

		return true;
	}

	return false;
}


PX_FORCE_INLINE void Shape::setControlStateIfExclusive(Scene* s, ControlState::Enum cs)
{
	if (isExclusive())
	{
		setControlState(cs);
		setScbScene(s);
	}
}


template<bool sync>
PX_FORCE_INLINE void Shape::checkUpdateOnRemove(Scene* s)
{
	// special code to cover the case where a shape has a pending update and gets released. The following operations have to be done
	// before the ref-counter of the shape gets decremented because that could cause the shape to be deleted in which case it must not
	// be in the pending update list any longer.
	if (getControlFlags() & Scb::ControlFlag::eIS_UPDATED)
	{
		if (sync)
			syncState();
		s->removeShapeFromPendingUpdateList(*this);
	}
}


//--------------------------------------------------------------
//
// Data synchronization
//
//--------------------------------------------------------------


}  // namespace Scb

}

#endif
