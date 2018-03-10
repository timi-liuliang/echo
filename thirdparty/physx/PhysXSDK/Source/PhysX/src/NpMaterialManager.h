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


#ifndef NP_MATERIALMANAGER
#define NP_MATERIALMANAGER

#include "NpMaterial.h"
#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

namespace physx
{
	class NpMaterialHandleManager
	{
	public:
		NpMaterialHandleManager() : mCurrentID(0), mFreeIDs(PX_DEBUG_EXP("NpMaterialHandleManager"))	{}

		void freeID(PxU32 id)
		{
			// Allocate on first call
			// Add released ID to the array of free IDs
			if(id == (mCurrentID - 1))
				--mCurrentID;
			else
				mFreeIDs.pushBack(id);
		}

		void freeAll()
		{
			mCurrentID = 0;
			mFreeIDs.clear();
		}

		PxU32 getNewID()
		{
			// If recycled IDs are available, use them
			const PxU32 size = mFreeIDs.size();
			if(size)
			{
				const PxU32 id = mFreeIDs[size-1]; // Recycle last ID
				mFreeIDs.popBack();
				return id;
			}
			// Else create a new ID
			return mCurrentID++;
		}

		PX_FORCE_INLINE PxU32 getNumMaterials()	const
		{
			return mCurrentID - mFreeIDs.size();
		}
	private:
		PxU32				mCurrentID;
		Ps::Array<PxU32>	mFreeIDs;
	};

	class NpMaterialManager 
	{
	public:
		NpMaterialManager()
		{
			const PxU32 matCount = 
#ifdef PX_PS3
			PX_PS3_MAX_MATERIAL_COUNT;
#else
			128;
#endif
			//we allocate +1 for a space for a dummy material for PS3 when we allocate a material > 128 and need to clear it immediately after
			mMaterials = (NpMaterial**)PX_ALLOC(sizeof(NpMaterial*) * matCount,  PX_DEBUG_EXP("NpMaterialManager::initialise"));
			mMaxMaterials = matCount;
			PxMemZero(mMaterials, sizeof(NpMaterial*)*mMaxMaterials);
		}

		~NpMaterialManager() {}

		void releaseMaterials()
		{
			for(PxU32 i=0; i<mMaxMaterials; ++i)
			{
				if(mMaterials[i])
				{
					const PxU32 handle = mMaterials[i]->getHandle();
					mHandleManager.freeID(handle);
					mMaterials[i]->release();
					mMaterials[i] = NULL;
				}
			}
			PX_FREE(mMaterials);
		}

		bool setMaterial(NpMaterial& mat)
		{
			const PxU32 materialIndex = mHandleManager.getNewID();

			if(materialIndex >= mMaxMaterials)
			{
#ifdef	PX_PS3
				mHandleManager.freeID(materialIndex);
				return false;
#else
				resize();
#endif
			}

			mMaterials[materialIndex] = &mat;
			mat.setHandle(materialIndex);
			return true;
		}

		void updateMaterial(NpMaterial& mat)
		{
			mMaterials[mat.getHandle()] = &mat;
		}

		PX_FORCE_INLINE PxU32 getNumMaterials()	const
		{
			return mHandleManager.getNumMaterials();
		}

		void removeMaterial(NpMaterial& mat)
		{
			const PxU32 handle = mat.getHandle();
			if(handle != MATERIAL_INVALID_HANDLE)
			{
				mMaterials[handle] = NULL;
				mHandleManager.freeID(handle);
			}
		}

		PX_FORCE_INLINE NpMaterial* getMaterial(const PxU32 index)const
		{
			PX_ASSERT(index <  mMaxMaterials);
			return mMaterials[index];
		}

		PX_FORCE_INLINE PxU32 getMaxSize()	const 
		{
			return mMaxMaterials;
		}

		PX_FORCE_INLINE NpMaterial** getMaterials() const
		{
			return mMaterials;
		}

	private:
		void resize()
		{
			const PxU32 numMaterials = mMaxMaterials;
			mMaxMaterials = mMaxMaterials*2;

			NpMaterial** mat = (NpMaterial**)PX_ALLOC(sizeof(NpMaterial*)*mMaxMaterials,  PX_DEBUG_EXP("NpMaterialManager::resize"));
			PxMemZero(mat, sizeof(NpMaterial*)*mMaxMaterials);
			for(PxU32 i=0; i<numMaterials; ++i)
			{
				mat[i] = mMaterials[i];
			}

			PX_FREE(mMaterials);

			mMaterials = mat;
		}

		NpMaterialHandleManager	mHandleManager;
		NpMaterial**			mMaterials;
		PxU32					mMaxMaterials;
		
	};

	class NpMaterialManagerIterator
	{
	public:
		NpMaterialManagerIterator(const NpMaterialManager& manager) : mManager(manager), mIndex(0)
		{
		}

		bool getNextMaterial(NpMaterial*& np)
		{
			const PxU32 maxSize = mManager.getMaxSize();
			PxU32 index = mIndex;
			while(index < maxSize && mManager.getMaterial(index)==NULL)
				index++;
			np = NULL;
			if(index < maxSize)
				np = mManager.getMaterial(index++);
			mIndex = index;
			return np!=NULL;
		}

		PxU32 getNumMaterials()	const
		{
			return mManager.getNumMaterials();
		}

	private:
		NpMaterialManagerIterator& operator=(const NpMaterialManagerIterator&);
		const NpMaterialManager&	mManager;
		PxU32						mIndex;
	};

}

#endif
