/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SAMPLE_APEX_RESOURCE_CALLBACK_H
#define SAMPLE_APEX_RESOURCE_CALLBACK_H

#include <NxApexDefs.h>
#include <NxApexSDK.h>
#if NX_SDK_VERSION_MAJOR == 2
#include <Nxp.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include <PxFiltering.h>
#endif
#include <NxResourceCallback.h>
#include <PxFileBuf.h>
#include "Find.h"
#include <SampleAsset.h>
#include <vector>

#pragma warning(push)
#pragma warning(disable:4512)

#if NX_SDK_VERSION_MAJOR == 3
class FilterBits; // forward reference the filter bits class
#endif

namespace physx
{
namespace apex
{
class NxApexSDK;
#if APEX_USE_PARTICLES
class NxModuleParticles;
#endif
}
}

namespace SampleRenderer
{
class Renderer;
}

namespace SampleFramework
{
class SampleAssetManager;
}

// TODO: DISABLE ME!!!
#define WORK_AROUND_BROKEN_ASSET_PATHS 1

enum SampleAssetFileType
{
	XML_ASSET,
	BIN_ASSET,
	ANY_ASSET,
};

class SampleApexResourceCallback : public physx::apex::NxResourceCallback
{
public:
	SampleApexResourceCallback(SampleRenderer::Renderer& renderer, SampleFramework::SampleAssetManager& assetManager);
	virtual				   ~SampleApexResourceCallback(void);

	void					addResourceSearchPath(const char* path);
	void					removeResourceSearchPath(const char* path);
	void					clearResourceSearchPaths();

#if NX_SDK_VERSION_MAJOR == 2
	void					registerNxCollisionGroup(const char* name, NxCollisionGroup group);
	void					registerNxGroupsMask128(const char* name, NxGroupsMask& groupsMask);
	void					registerNxCollisionGroupsMask(const char* name, physx::PxU32 groupsBitMask);
	void					registerPhysicalMaterial(const char* name, NxMaterialIndex physicalMaterial);
#elif NX_SDK_VERSION_MAJOR == 3
	void					registerSimulationFilterData(const char* name, const physx::PxFilterData& simulationFilterData);
	void					registerPhysicalMaterial(const char* name, physx::PxMaterialTableIndex physicalMaterial);
#endif

	void					registerNxGroupsMask64(const char* name, physx::apex::NxGroupsMask64& groupsMask);

	void					setApexSupport(physx::apex::NxApexSDK& apexSDK);

	physx::PxFileBuf*   	findApexAsset(const char* assetName);
	void					findFiles(const char* dir, physx::apex::FileHandler& handler);

	void					setAssetPreference(SampleAssetFileType pref)
	{
		m_assetPreference = pref;
	}

	static bool				xmlFileExtension(const char* assetName);
	static const char*		getFileExtension(const char* assetName);

private:
	SampleFramework::SampleAsset*	findSampleAsset(const char* assetName, SampleFramework::SampleAsset::Type type);

#if WORK_AROUND_BROKEN_ASSET_PATHS
	const char*				mapHackyPath(const char* path);
#endif

public:
	virtual void*			requestResource(const char* nameSpace, const char* name);
	virtual void			releaseResource(const char* nameSpace, const char* name, void* resource);

	bool					doesFileExist(const char* filename, const char* ext);
	bool					doesFileExist(const char* filename);
	bool					isFileReadable(const char* fullPath);

protected:
	SampleRenderer::Renderer&				m_renderer;
	SampleFramework::SampleAssetManager&	m_assetManager;
	std::vector<char*>						m_searchPaths;
#if NX_SDK_VERSION_MAJOR == 2
	std::vector<NxGroupsMask>				m_nxGroupsMasks;
#elif NX_SDK_VERSION_MAJOR == 3
	std::vector<physx::PxFilterData>		m_FilterDatas;
	FilterBits								*m_FilterBits;
#endif
	std::vector<physx::apex::NxGroupsMask64>	m_nxGroupsMask64s;
#if APEX_USE_PARTICLES
	physx::apex::NxModuleParticles*			mModuleParticles;
#endif
	physx::apex::NxApexSDK*					m_apexSDK;
	physx::PxU32							m_numGets;
	SampleAssetFileType						m_assetPreference;
};

#pragma warning(pop)

#endif // SAMPLE_APEX_RESOURCE_CALLBACK_H
