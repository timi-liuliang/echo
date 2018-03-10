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


#ifndef PVD_VISUALDEBUGGER_H
#define PVD_VISUALDEBUGGER_H

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "PsUserAllocated.h"
#include "PxVisualDebugger.h"
#include "PvdConnectionManager.h"
#include "CmPhysXCommon.h"
#include "PxMetaDataPvdBinding.h"
#include "NpFactory.h"
#include "PxPhysXConfig.h"

namespace physx
{

namespace Scb
{
	class Scene;
	class Body;
	class RigidStatic;
	class RigidObject;
	class Shape;
}
class PxGeometry;

namespace Sc
{
	class RigidCore;
}
class PxScene;
class PxTriangleMesh;
class PxConvexMesh;
class PxHeightField;
class PxClothFabric;

namespace Pvd
{

	class PvdMetaDataBinding;

struct SdkGroups
{
	enum Enum
	{
		Scenes,
		TriangleMeshes,
		ConvexMeshes,
		HeightFields,
		ClothFabrics,
		NUM_ELEMENTS
	};
};


//////////////////////////////////////////////////////////////////////////
/*!
RemoteDebug supplies functionality for writing debug info to a stream
to be read by the remote debugger application.
*/
//////////////////////////////////////////////////////////////////////////
class VisualDebugger : public PxVisualDebugger, public Ps::UserAllocated, public physx::debugger::comm::PvdConnectionHandler, public Pvd::BufferRegistrar, public NpFactoryListener
{
	PX_NOCOPY(VisualDebugger)
public:
	VisualDebugger ();
	virtual ~VisualDebugger ();
	virtual void disconnect();
	virtual physx::debugger::comm::PvdConnection* getPvdConnection();
	virtual physx::debugger::comm::PvdDataStream* getPvdDataStream(const PxScene& scene);
	virtual void updateCamera(const char* name, const PxVec3& origin, const PxVec3& up, const PxVec3& target);
	virtual void sendErrorMessage(PxErrorCode::Enum code, const char* message, const char* file, PxU32 line);
	virtual void setVisualDebuggerFlag(PxVisualDebuggerFlag::Enum flag, bool value);
	virtual void setVisualDebuggerFlags(PxVisualDebuggerFlags flags);
	virtual PxU32 getVisualDebuggerFlags();

	// internal methods
	void setCreateContactReports(bool value);

	void sendClassDescriptions();

	// useCachedStaus: 
	//    1> When useCachedStaus is false, isConnected() checks the lowlevel network status.
	//       This can be slow because it needs to lock the lowlevel network stream. If isConnected() is 
	//       called frequently, the expense of locking can be significant.
	//    2> When useCachedStaus is true, isConnected() checks the highlevel cached status with atomic access.
	//       It is faster than locking, but the status may be different from the lowlevel network with latency of up to one frame.
	//       The reason for this is that the cached status is changed inside PvdConnectionHandler listener, which is not called immediately
	//       when the lowlevel conntion status changes. 
	bool isConnected(bool useCachedStatus = false);

	void checkConnection();
	void updateScenesPvdConnection();
	void setupSceneConnection(Scb::Scene& s);

	void sendEntireSDK();
	void createGroups();
	void releaseGroups();

	template<typename TDataType>
	inline void increaseReference(const TDataType* inItem)		{ if(incRef(inItem) == 1) { createPvdInstance(inItem); flush(); } }

	template<typename TDataType>
	inline void decreaseReference(const TDataType* inItem)		{ if(decRef(inItem) == 0) { destroyPvdInstance(inItem); flush(); } }

	PX_FORCE_INLINE bool	getTransmitContactsFlag()							{ return (mFlags & PxVisualDebuggerFlag::eTRANSMIT_CONTACTS) != 0; }
	PX_FORCE_INLINE bool	getTransmitSceneQueriesFlag()						{ return (mFlags & PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES) != 0; }
	

	static PX_FORCE_INLINE const char* getPhysxNamespace() { return "physx3"; }

	void updatePvdProperties(const PxMaterial* mat);

	// physx::debugger::PvdConnectionHandler
	virtual void onPvdSendClassDescriptions( physx::debugger::comm::PvdConnection& inFactory );
	virtual void onPvdConnected( physx::debugger::comm::PvdConnection& inFactory );
	virtual void onPvdDisconnected( physx::debugger::comm::PvdConnection& inFactory );
	// physx::debugger::PvdConnectionHandler

	//Pvd::BufferRegistrar
	virtual void addRef( const PxMaterial* buffer ) { increaseReference( buffer ); }
	virtual void addRef( const PxConvexMesh* buffer ) { increaseReference( buffer ); }
	virtual void addRef( const PxTriangleMesh* buffer ) { increaseReference( buffer ); }
	virtual void addRef( const PxHeightField* buffer ) { increaseReference( buffer ); }
#if PX_USE_CLOTH_API
	virtual void addRef( const PxClothFabric* buffer ) { increaseReference( buffer ); }
#endif
	//Pvd::BufferRegistrar

	
	//NpFactoryListener
	virtual void onGuMeshFactoryBufferRelease(const PxBase* object, PxType typeID, bool memRelease);
#if PX_USE_CLOTH_API
	virtual void onNpFactoryBufferRelease(PxClothFabric& data);
#endif
	///NpFactoryListener
	
private:
	template<typename TDataType> void doMeshFactoryBufferRelease( const TDataType* type );
	void createPvdInstance(const PxTriangleMesh* triMesh);
	void destroyPvdInstance(const PxTriangleMesh* triMesh);
	void createPvdInstance(const PxConvexMesh* convexMesh);
	void destroyPvdInstance(const PxConvexMesh* convexMesh);
	void createPvdInstance(const PxHeightField* heightField);
	void destroyPvdInstance(const PxHeightField* heightField);
	void createPvdInstance(const PxMaterial* mat);
	void destroyPvdInstance(const PxMaterial* mat);
#if PX_USE_CLOTH_API
	void createPvdInstance(const PxClothFabric* fabric);
	void destroyPvdInstance(const PxClothFabric* fabric);
#endif
	void flush();

	PX_FORCE_INLINE PxU32 incRef(const void* ptr);
	PX_FORCE_INLINE PxU32 decRef(const void* ptr);


	physx::debugger::comm::PvdDataStream*				mPvdDataStream;
	physx::debugger::comm::PvdConnection*				mPvdConnection;
	PvdMetaDataBinding				mMetaDataBinding;

	Ps::HashMap<const void*, PxU32>	mRefCountMap;
	Ps::Mutex						mRefCountMapLock;

	PxU32							mFlags;
	volatile PxI32					mIsConnected;
};


PX_FORCE_INLINE PxU32 VisualDebugger::incRef(const void* ptr)
{
	Ps::Mutex::ScopedLock lock(mRefCountMapLock);

	if(mRefCountMap.find(ptr))
	{
		PxU32& counter = mRefCountMap[ptr];
		counter++;
		return counter;
	}
	else
	{
		mRefCountMap.insert(ptr, 1);
		return 1;
	}
}

PX_FORCE_INLINE PxU32 VisualDebugger::decRef(const void* ptr)
{
	Ps::Mutex::ScopedLock lock(mRefCountMapLock);
	const Ps::HashMap<const void*, PxU32>::Entry* entry = mRefCountMap.find(ptr);
	if ( entry )
	{
		PxU32& retval( const_cast<PxU32&>( entry->second ) );
		if ( retval )
			--retval;
		PxU32 theValue = retval;
		if ( !theValue )
			mRefCountMap.erase( ptr );
		return theValue;
	}
	return PX_MAX_U32;
}


} // namespace Pvd

}

#endif // PX_SUPPORT_VISUAL_DEBUGGER

#endif // VISUALDEBUGGER_H

