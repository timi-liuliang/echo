/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_RENDERMESH_ASSET_H
#define APEX_RENDERMESH_ASSET_H

#include "NiApexRenderMeshAsset.h"
#include "ApexInterface.h"
#include "ApexSDKHelpers.h"
#include "NiResourceProvider.h"
#include "ApexRenderSubmesh.h"

#include "RenderMeshAssetParameters.h"
#include "ApexRWLockable.h"

namespace physx
{
namespace apex
{

/**
ApexRenderMeshAsset - a collection of ApexRenderMeshParts and submesh extra data
*/
class ApexRenderMeshAsset : public NiApexRenderMeshAsset, public NxApexResource, public ApexRWLockable, public ApexResource
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexRenderMeshAsset(NxResourceList& list, const char* name, NxAuthObjTypeID ownerModuleID);
	~ApexRenderMeshAsset();

	struct SubmeshData
	{
		NxUserRenderVertexBuffer* staticVertexBuffer;
		NxUserRenderVertexBuffer* skinningVertexBuffer;
		NxUserRenderVertexBuffer* dynamicVertexBuffer;
		bool needsStaticData;
		bool needsDynamicData;
	};
	struct CustomSubmeshData
	{
		Array<NxRenderDataFormat::Enum>	customBufferFormats;
		Array<void*>					customBufferVoidPtrs;
	};


	void						release()
	{
		NxGetApexSDK()->releaseAsset(*this);
	}

	void						destroy();

	NxAuthObjTypeID				getObjTypeID() const
	{
		return mObjTypeID;
	}
	const char* 				getObjTypeName() const
	{
		return getClassName();
	}
	PxU32						forceLoadAssets();
	void						deleteStaticBuffersAfterUse(bool set)
	{
		mParams->deleteStaticBuffersAfterUse = set;
	}

	NxRenderMeshActor* 			createActor(const NxRenderMeshActorDesc& desc);
	void                        releaseActor(NxRenderMeshActor& renderMeshActor);

	const char* 				getName(void) const
	{
		return mName.c_str();
	}
	PxU32						getSubmeshCount() const
	{
		return (physx::PxU32)mParams->submeshes.arraySizes[0];
	}
	PxU32						getPartCount() const
	{
		return (physx::PxU32)mParams->partBounds.arraySizes[0];
	}
	PxU32						getBoneCount() const
	{
		return mParams->boneCount;
	}
	const NxRenderSubmesh&		getSubmesh(PxU32 submeshIndex) const
	{
		return *mSubmeshes[submeshIndex];
	}
	const PxBounds3&			getBounds(PxU32 partIndex = 0) const
	{
		return mParams->partBounds.buf[partIndex];
	}
	void						getStats(NxRenderMeshAssetStats& stats) const;

	// from NiApexRenderMeshAsset
	NiApexRenderSubmesh&		getNiSubmesh(PxU32 submeshIndex)
	{
		return *mSubmeshes[submeshIndex];
	}
	void						permuteBoneIndices(const Array<PxI32>& old2new);
	void						applyTransformation(const PxMat34Legacy& transformation, PxF32 scale);
	void						reverseWinding();
	void						applyScale(PxF32 scale);
	bool						mergeBinormalsIntoTangents();
	void						setOwnerModuleId(NxAuthObjTypeID id)
	{
		mOwnerModuleID = id;
	}
	NxTextureUVOrigin::Enum		getTextureUVOrigin() const;

	const char* 				getMaterialName(PxU32 submeshIndex) const
	{
		return mParams->materialNames.buf[submeshIndex];
	}

	// NxApexResource methods
	void						setListIndex(NxResourceList& list, PxU32 index)
	{
		m_listIndex = index;
		m_list = &list;
	}
	PxU32						getListIndex() const
	{
		return m_listIndex;
	}

	/* Common data for all ApexRenderMeshAssets */
	static NxAuthObjTypeID		mObjTypeID;
	static const char*          getClassName()
	{
		return NX_RENDER_MESH_AUTHORING_TYPE_NAME;
	};

	const NxParameterized::Interface* getAssetNxParameterized() const
	{
		return mParams;
	}

	/**
	 * \brief Releases the ApexAsset but returns the NxParameterized::Interface and *ownership* to the caller.
	 */
	virtual NxParameterized::Interface* releaseAndReturnNxParameterizedInterface(void)
	{
		NxParameterized::Interface* ret = mParams;
		mParams = NULL;
		release();
		return ret;
	}
	NxParameterized::Interface* getDefaultActorDesc()
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	};

	NxParameterized::Interface* getDefaultAssetPreviewDesc()
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	};

	virtual NxApexActor* createApexActor(const NxParameterized::Interface& /*parms*/, NxApexScene& /*apexScene*/)
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	virtual NxApexAssetPreview* createApexAssetPreview(const NxParameterized::Interface& /*params*/, NxApexAssetPreviewScene* /*previewScene*/)
	{
		APEX_INVALID_OPERATION("Not yet implemented!");
		return NULL;
	}

	void setOpaqueMesh(NxUserOpaqueMesh* om)
	{
		mOpaqueMesh = om;
	}

	NxUserOpaqueMesh* getOpaqueMesh(void) const
	{
		return mOpaqueMesh;
	}

	virtual bool isValidForActorCreation(const ::NxParameterized::Interface& /*parms*/, NxApexScene& /*apexScene*/) const
	{
		return true; // TODO implement this method
	}

	virtual bool isDirty() const
	{
		return false;
	}

protected:
	ApexRenderMeshAsset() {}	// Nop constructor for use by NxRenderMeshAssetAuthoring

	void						setSubmeshCount(PxU32 submeshCount);


	void						createLocalData();

	bool						createFromParameters(RenderMeshAssetParameters* params);

	void						updatePartBounds();

	NxAuthObjTypeID				mOwnerModuleID;
	RenderMeshAssetParameters*	mParams;
	NxUserOpaqueMesh*			mOpaqueMesh;

	// Name should not be serialized
	ApexSimpleString			mName;

	// Local (not serialized) data
	Array<ApexRenderSubmesh*>	mSubmeshes;

	Array<NxResID>				mMaterialIDs;
	NxResourceList				mActorList;
	Array<SubmeshData>			mRuntimeSubmeshData;
	Array<CustomSubmeshData>	mRuntimeCustomSubmeshData;

	friend class ApexRenderMeshActor;
	friend class RenderMeshAuthorableObject;
};

} // namespace apex
} // namespace physx

#endif // APEX_RENDERMESH_ASSET_H
