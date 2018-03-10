/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __NI_IOFX_MANAGER_H__
#define __NI_IOFX_MANAGER_H__

#include "PsArray.h"
#include "foundation/PxVec3.h"
#include "foundation/PxVec4.h"
#include "PxMat33Legacy.h"

namespace physx
{

typedef PxU32 PxTaskID;
class PxGpuCopyDescQueue;

namespace apex
{
class NxIofxAsset;
class NxApexRenderVolume;

namespace iofx
{
#ifdef APEX_TEST
struct IofxManagerTestData;
#endif
}

template <class T>
class ApexMirroredArray;


class NiIofxManagerDesc
{
public:
	NiIofxManagerDesc() :
		iosAssetName(NULL),
		iosOutputsOnDevice(false),
		iosSupportsDensity(false),
		iosSupportsCollision(false),
		iosSupportsUserData(false),
		maxObjectCount(0),
		maxInputCount(0),
		maxInStateCount(0)
	{
	}

	const char* iosAssetName;
	bool		iosOutputsOnDevice;
	bool		iosSupportsDensity;
	bool		iosSupportsCollision;
	bool		iosSupportsUserData;
	PxU32		maxObjectCount;
	PxU32		maxInputCount;
	PxU32		maxInStateCount;
};

/// The IOFX will update the volumeID each simulation step, the IOS must
/// persist this output. IOS provides initial volumeID, based on emitter's
/// preferred volume.
struct NiIofxActorID
{
	PxU32		value;

	PX_CUDA_CALLABLE PX_INLINE NiIofxActorID() {}
	PX_CUDA_CALLABLE PX_INLINE explicit NiIofxActorID(PxU32 arg)
	{
		value = arg;
	}

	PX_CUDA_CALLABLE PX_INLINE void set(PxU16 volumeID, PxU16 actorClassID)
	{
		value = (PxU32(volumeID) << 16) | PxU32(actorClassID);
	}

	PX_CUDA_CALLABLE PX_INLINE PxU16 getVolumeID() const
	{
		return PxU16(value >> 16);
	}
	PX_CUDA_CALLABLE PX_INLINE void setVolumeID(PxU16 volumeID)
	{
		value &= 0x0000FFFFu;
		value |= (PxU32(volumeID) << 16);
	}

	PX_CUDA_CALLABLE PX_INLINE PxU16 getActorClassID() const
	{
		return PxU16(value & 0xFFFFu);
	}
	PX_CUDA_CALLABLE PX_INLINE void setActorClassID(PxU16 actorClassID)
	{
		value &= 0xFFFF0000u;
		value |= PxU32(actorClassID);
	}

	static const physx::PxU16 NO_VOLUME = 0xFFFFu;
	static const physx::PxU16 INV_ACTOR = 0xFFFFu;
};


/* IOFX Manager returned pointers for simulation data */
class NiIosBufferDesc
{
public:
	/* All arrays are indexed by input ID */
	ApexMirroredArray<PxVec4>*			pmaPositionMass;
	ApexMirroredArray<PxVec4>*			pmaVelocityLife;
	ApexMirroredArray<PxVec4>*			pmaCollisionNormalFlags;
	ApexMirroredArray<PxF32>*			pmaDensity;
	ApexMirroredArray<NiIofxActorID>*	pmaActorIdentifiers;
	ApexMirroredArray<PxU32>*			pmaInStateToInput;
	ApexMirroredArray<PxU32>*			pmaOutStateToInput;

	ApexMirroredArray<PxU32>*			pmaUserData;

	//< Value in inStateToInput field indicates a dead particle, input to IOFX
	static const physx::PxU32 NOT_A_PARTICLE = 0xFFFFFFFFu;

	//< Flag in inStateToInput field indicates a new particle, input to IOFX
	static const physx::PxU32 NEW_PARTICLE_FLAG = 0x80000000u;
};

// This is a representative of uint4 on host
struct IofxSlice
{
	PxU32 x, y, z, w;
};

typedef void (*EventCallback)(void*);

class NiIofxManagerCallback
{
public:
	virtual void operator()(void* stream = NULL) = 0;
};

class NiIofxManagerClient
{
public:
	struct Params
	{
		PxF32 objectScale;

		Params()
		{
			setDefaults();
		}

		void setDefaults()
		{
			objectScale = 1.0f;
		}
	};
	virtual void getParams(NiIofxManagerClient::Params& params) const = 0;
	virtual void setParams(const NiIofxManagerClient::Params& params) = 0;
};


class NiIofxManager
{
public:
	//! An IOS Actor will call this once, at creation
	virtual void createSimulationBuffers(NiIosBufferDesc& outDesc) = 0;

	//! An IOS actor will call this once, when it creates its fluid simulation
	virtual void setSimulationParameters(PxF32 radius, const PxVec3& up, PxF32 gravity, PxF32 restDensity) = 0;

	//! An IOS Actor will call this method after each simulation step
	virtual void updateEffectsData(PxF32 deltaTime, PxU32 numObjects, PxU32 maxInputID, PxU32 maxStateID, void* extraData = 0) = 0;

	//! An IOS Actor will call this method at the start of each step IOFX will run
	virtual PxTaskID getUpdateEffectsTaskID(PxTaskID) = 0;

	virtual PxU16 getActorClassID(NiIofxManagerClient* client, PxU16 meshID) = 0;

	virtual NiIofxManagerClient* createClient(physx::apex::NxIofxAsset* asset, const NiIofxManagerClient::Params& params) = 0;
	virtual void releaseClient(NiIofxManagerClient* client) = 0;

	virtual PxU16 getVolumeID(physx::apex::NxApexRenderVolume* vol) = 0;

	//! Triggers the IOFX Manager to copy host buffers to the device
	//! This is intended for use in an IOS post-update task, if they
	//! need the output buffers on the device.
	virtual void outputHostToDevice(physx::PxGpuCopyDescQueue& copyQueue) = 0;
	
	//! NiIofxManagerCallback will be called before Iofx computations
	virtual void setOnStartCallback(NiIofxManagerCallback*) = 0;
	//! NiIofxManagerCallback will be called after Iofx computations
	virtual void setOnFinishCallback(NiIofxManagerCallback*) = 0;

	//! Called when IOS is being deleted
	virtual void release() = 0;

	//get bounding box
	virtual PxBounds3 getBounds() const = 0;

#ifdef APEX_TEST
	virtual iofx::IofxManagerTestData* createTestData() = 0;
	virtual void copyTestData() const = 0;
	virtual void clearTestData() = 0;
#endif
protected:
	virtual ~NiIofxManager() {}
};


}
} // end namespace physx::apex

#endif // __NI_IOFX_MANAGER_H__
