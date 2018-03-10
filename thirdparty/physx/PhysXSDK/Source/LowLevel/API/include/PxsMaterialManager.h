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


#ifndef PXS_MATERIALMANAGER
#define PXS_MATERIALMANAGER

#include "PxsMaterialCore.h"
#include "PsAlignedMalloc.h"
#ifdef PX_PS3
#include "ps3/PxPS3Config.h"
#endif

namespace physx
{
	struct PxsMaterialInfo
	{
		PxU16 mMaterialIndex0;
		PxU16 mMaterialIndex1;
	};

	class PxsMaterialManager 
	{
	public:
		PxsMaterialManager()
		{
#ifndef __SPU__
			const PxU32 matCount = 
#ifdef PX_PS3
			PX_PS3_MAX_MATERIAL_COUNT;
#else
			128;
#endif
			materials = (PxsMaterialCore*)physx::shdfnd::AlignedAllocator<16>().allocate(sizeof(PxsMaterialCore)*matCount,  __FILE__, __LINE__);
			maxMaterials = matCount;
			for(PxU32 i=0; i<matCount; ++i)
			{
				materials[i].setMaterialIndex(MATERIAL_INVALID_HANDLE);
			}
#endif
		}

		~PxsMaterialManager()
		{
#ifndef __SPU__
			physx::shdfnd::AlignedAllocator<16>().deallocate(materials);
#endif
		}

		void setMaterial(PxsMaterialCore* mat)
		{
			const PxU32 materialIndex = mat->getMaterialIndex();
			resize(materialIndex+1);
			materials[materialIndex] = *mat;
		}

		void updateMaterial(PxsMaterialCore* mat)
		{
			materials[mat->getMaterialIndex()] =*mat;
		}

		void removeMaterial(PxsMaterialCore* mat)
		{
			mat->setMaterialIndex(MATERIAL_INVALID_HANDLE);
		}

		PX_FORCE_INLINE PxsMaterialCore* getMaterial(const PxU32 index)const
		{
			PX_ASSERT(index <  maxMaterials);
			return &materials[index];
		}

		PxU32 getMaxSize()const 
		{
			return maxMaterials;
		}

		void resize(PxU32 minValueForMax)
		{
#ifndef __SPU__
			if(maxMaterials>=minValueForMax)
				return;

			const PxU32 numMaterials = maxMaterials;
			
			maxMaterials = (minValueForMax+31)&~31;
			PxsMaterialCore* mat = (PxsMaterialCore*)physx::shdfnd::AlignedAllocator<16>().allocate(sizeof(PxsMaterialCore)*maxMaterials,  __FILE__, __LINE__);
			for(PxU32 i=0; i<numMaterials; ++i)
			{
				mat[i] = materials[i];
			}
			for(PxU32 i = numMaterials; i < maxMaterials; ++i)
			{
				mat[i].setMaterialIndex(MATERIAL_INVALID_HANDLE);
			}

			physx::shdfnd::AlignedAllocator<16>().deallocate(materials);

			materials = mat;
#endif
		}

		PxsMaterialCore* materials;//make sure materials's start address is 16 bytes align
		PxU32 maxMaterials;
		PxU32 mPad[2];
	};

	class PxsMaterialManagerIterator
	{
	public:
		PxsMaterialManagerIterator(PxsMaterialManager& manager) : mManager(manager), mIndex(0)
		{
		}

		bool getNextMaterial(PxsMaterialCore*& materialCore)
		{
			const PxU32 maxSize = mManager.getMaxSize();
			PxU32 index = mIndex;
			while(index < maxSize && mManager.getMaterial(index)->getMaterialIndex() == MATERIAL_INVALID_HANDLE)
				index++;
			materialCore = NULL;
			if(index < maxSize)
				materialCore = mManager.getMaterial(index++);
			mIndex = index;
			return materialCore!=NULL;
		}

	private:
		PxsMaterialManagerIterator& operator=(const PxsMaterialManagerIterator&);
		PxsMaterialManager& mManager;
		PxU32				mIndex;
	};

}

#endif
