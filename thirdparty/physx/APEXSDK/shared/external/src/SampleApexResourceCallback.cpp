/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "NxApexDefs.h"
#include "SampleApexResourceCallback.h"

#include <NxApexNameSpace.h>
#include <NxApexSDK.h>
#include <NxResourceProvider.h>

#include <NxDestructibleAsset.h>
#include <NxClothingAsset.h>

#if APEX_USE_PARTICLES
#include <NxApexEmitterAsset.h>
#include <NxBasicIOSAsset.h>
#include <NxBasicFSAsset.h>
#include <NxExplosionAsset.h>
#include <NxFieldBoundaryAsset.h>
#include <NxGroundEmitterAsset.h>
#include <NxIofxAsset.h>
#include <NxImpactEmitterAsset.h>
#include <NxTurbulenceFSAsset.h>
#include <NxVelocitySourceAsset.h>
#include <NxHeatSourceAsset.h>
#include <NxSubstanceSourceAsset.h>
#include <NxModuleParticles.h>

#if NX_SDK_VERSION_MAJOR == 2
#include <NxFluidIosAsset.h>
#endif

#if NX_SDK_VERSION_MAJOR == 3
#include <NxParticleIosAsset.h>
#include <NxForceFieldAsset.h>
#endif
#endif // APEX_USE_PARTICLES

#if NX_SDK_VERSION_MAJOR == 3
#include "FilterBits.h"
#endif

#include <SampleAssetManager.h>
#include <SampleMaterialAsset.h>
#include <Renderer.h>

#include <PsString.h>
#include <PsUtilities.h>

#define INPLACE_BINARY 1

#define DEBUG_RESOURCE_REQUESTS 0
#if DEBUG_RESOURCE_REQUESTS
# include <stdio.h>
# define DEBUG_OUTPUT_FILENAME "debugResourceRequests.txt"
FILE* gDebugOutput = 0 /* stdout */;
#endif

#include <sys/stat.h>

/*****************************
* SampleApexResourceCallback *
*****************************/

SampleApexResourceCallback::SampleApexResourceCallback(SampleRenderer::Renderer& renderer, SampleFramework::SampleAssetManager& assetManager) :
	m_renderer(renderer),
	m_assetManager(assetManager),
	m_assetPreference(ANY_ASSET)
{
#if APEX_USE_PARTICLES
	mModuleParticles			= NULL;
#endif
	m_apexSDK                   = 0;
	m_numGets                   = 0;

#if NX_SDK_VERSION_MAJOR == 2
	// if this resizes it will cause problems
	// APEX uses pointers from this list
	m_nxGroupsMasks.reserve(128);
#elif NX_SDK_VERSION_MAJOR == 3
	m_FilterDatas.reserve(128);
	m_FilterBits = createFilterBits(); // create the weighted collision filtering helper class
#endif
	m_nxGroupsMask64s.reserve(128);

#if DEBUG_RESOURCE_REQUESTS
	if (!gDebugOutput)
	{
		gDebugOutput = fopen(DEBUG_OUTPUT_FILENAME, "w");
	}
#endif
}

SampleApexResourceCallback::~SampleApexResourceCallback(void)
{
	PX_ASSERT(m_numGets == 0);
	clearResourceSearchPaths();
#if NX_SDK_VERSION_MAJOR == 3
	if ( m_FilterBits )
	{
		m_FilterBits->release();
	}
#endif
#if DEBUG_RESOURCE_REQUESTS
	if (gDebugOutput && gDebugOutput != stdout && gDebugOutput != stderr)
	{
		fclose(gDebugOutput);
	}
#endif
}

void SampleApexResourceCallback::addResourceSearchPath(const char* path)
{
	physx::PxU32 len = path && *path ? (physx::PxU32)strlen(path) : 0;
	if (len)
	{
		len++;
		char* searchPath = new char[len];
		physx::string::strncpy_s(searchPath, len, path, len);
		m_searchPaths.push_back(searchPath);
	}
}

void SampleApexResourceCallback::removeResourceSearchPath(const char* path)
{
	if (path)
	{
		for (physx::PxU32 i = 0; i < m_searchPaths.size(); i++)
		{
			if (strcmp(m_searchPaths[i], path) == 0)
			{
				m_searchPaths.erase(m_searchPaths.begin() + i);
				break;
			}
		}
	}
}

void SampleApexResourceCallback::clearResourceSearchPaths()
{
	const size_t numSearchPaths = m_searchPaths.size();
	for (size_t i = 0; i < numSearchPaths; i++)
	{
		delete [] m_searchPaths[i];
	}
	m_searchPaths.clear();
}

#if NX_SDK_VERSION_MAJOR == 2
void SampleApexResourceCallback::registerNxCollisionGroup(const char* name, NxCollisionGroup group)
{
	PX_ASSERT(m_apexSDK);
	if (m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_NAME_SPACE, name, (void*)group);
	}
}

void SampleApexResourceCallback::registerNxGroupsMask128(const char* name, NxGroupsMask& groupsMask)
{
	PX_ASSERT(m_apexSDK);
	PX_ASSERT(m_nxGroupsMasks.size() < 128);
	if (m_apexSDK)
	{
		m_nxGroupsMasks.push_back(groupsMask);
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_128_NAME_SPACE, name, (void*)(&(m_nxGroupsMasks.back())));
	}
}

void SampleApexResourceCallback::registerNxCollisionGroupsMask(const char* name, physx::PxU32 groupsBitMask)
{
	PX_ASSERT(m_apexSDK);
	if (m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_MASK_NAME_SPACE, name, (void*)(physx::PxU64)groupsBitMask);
	}
}

void SampleApexResourceCallback::registerPhysicalMaterial(const char* name, NxMaterialIndex physicalMaterial)
{
	PX_ASSERT(m_apexSDK);
	if (m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_PHYSICS_MATERIAL_NAME_SPACE, name, (void*)physicalMaterial);
	}
}
#elif NX_SDK_VERSION_MAJOR == 3
void SampleApexResourceCallback::registerPhysicalMaterial(const char* name, physx::PxMaterialTableIndex physicalMaterial)
{
	PX_ASSERT(m_apexSDK);
	if (m_apexSDK)
	{
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_PHYSICS_MATERIAL_NAME_SPACE, name, (void*)(static_cast<size_t>(physicalMaterial)));
	}
}
void SampleApexResourceCallback::registerSimulationFilterData(const char* name, const physx::PxFilterData& simulationFilterData)
{
	PX_ASSERT(m_apexSDK);
	PX_ASSERT(m_FilterDatas.size() < 128);
	if (m_apexSDK)
	{
		m_FilterDatas.push_back(simulationFilterData);
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_128_NAME_SPACE, name, (void*)&m_FilterDatas.back());
	}
}
#endif

void SampleApexResourceCallback::registerNxGroupsMask64(const char* name, physx::apex::NxGroupsMask64& groupsMask)
{
	PX_ASSERT(m_apexSDK);
	PX_ASSERT(m_nxGroupsMask64s.size() < 128);
	if (m_apexSDK)
	{
		m_nxGroupsMask64s.push_back(groupsMask);
		m_apexSDK->getNamedResourceProvider()->setResource(APEX_COLLISION_GROUP_64_NAME_SPACE, name, (void*)(&(m_nxGroupsMask64s.back())));
	}
}


//NxGroupsMask64
void SampleApexResourceCallback::setApexSupport(physx::apex::NxApexSDK& apexSDK)
{
	PX_ASSERT(!m_apexSDK);
	m_apexSDK	= &apexSDK;
#if APEX_USE_PARTICLES
	physx::PxU32 count = m_apexSDK->getNbModules();
	physx::apex::NxModule **modules = m_apexSDK->getModules();
	for (physx::PxU32 i=0; i<count; i++)
	{
		physx::apex::NxModule *m = modules[i];
		const char *name = m->getName();
		if ( strcmp(name,"Particles") == 0 )
		{
			mModuleParticles = static_cast< physx::apex::NxModuleParticles *>(m);
			break;
		}
	}
#endif
}

physx::PxFileBuf* SampleApexResourceCallback::findApexAsset(const char* assetName)
{
	physx::PxFileBuf* outStream = 0;
	const size_t numSearchPaths = m_searchPaths.size();
	for (size_t i = 0; i < numSearchPaths; i++)
	{
		const char* searchPath = m_searchPaths[i];
		const physx::PxU32 pathMaxLen = 512;
		char        fullPath[pathMaxLen] = {0};
		physx::string::strcpy_s(fullPath, pathMaxLen, searchPath);
		physx::string::strcat_s(fullPath, pathMaxLen, assetName);

		outStream = m_apexSDK->createStream(fullPath, physx::PxFileBuf::OPEN_READ_ONLY);
		if (outStream && outStream->getOpenMode() == physx::PxFileBuf::OPEN_READ_ONLY)
		{
			break;
		}
		else if (outStream)
		{
			outStream->release();
			outStream = 0;
		}
	}
	PX_ASSERT(outStream);
	return outStream;
}

void SampleApexResourceCallback::findFiles(const char* dir, physx::apex::FileHandler& handler)
{
	const unsigned int fullMaskLength = 256;
	char fullMask[fullMaskLength];

	for (size_t i = 0; i < m_searchPaths.size(); i++)
	{
		physx::string::strcpy_s(fullMask, fullMaskLength, m_searchPaths[i]);
		physx::string::strcat_s(fullMask, fullMaskLength, dir);

		Find(fullMask, handler);
	}
}

bool SampleApexResourceCallback::doesFileExist(const char* filename, const char* ext)
{
	char fullname[512] = {0};
	physx::string::strcat_s(fullname, sizeof(fullname), filename);
	physx::string::strcat_s(fullname, sizeof(fullname), ext);

	return doesFileExist(fullname);
}

bool SampleApexResourceCallback::doesFileExist(const char* filename)
{
	const size_t numSearchPaths = m_searchPaths.size();
	for (size_t i = 0; i < numSearchPaths; i++)
	{
		const char* searchPath = m_searchPaths[i];
		const physx::PxU32 pathMaxLen = 512;
		char        fullPath[pathMaxLen] = {0};
		physx::string::strcpy_s(fullPath, pathMaxLen, searchPath);
		physx::string::strcat_s(fullPath, pathMaxLen, filename);

#ifdef PX_X360
		// exchange '/' with '\'
		for (unsigned int i = 0, len = strlen(fullPath); i < len; i++)
		{
			if (fullPath[i] == '/')
			{
				fullPath[i] = '\\';
			}
		}
#endif
		struct stat info;		
		if ( (stat(fullPath, &info) != -1) && (info.st_mode & (S_IFREG)) != 0)
		{
			return true;
		}
	}

	return false;
}

bool SampleApexResourceCallback::isFileReadable(const char* fullPath)
{
	physx::PxFileBuf* outStream = m_apexSDK->createStream(fullPath, physx::PxFileBuf::OPEN_READ_ONLY);
	bool isSuccess = false;
	if (outStream != NULL)
	{
		isSuccess = outStream->getOpenMode() == physx::PxFileBuf::OPEN_READ_ONLY;
		outStream->release();		
	}
	return isSuccess;
}

SampleFramework::SampleAsset* SampleApexResourceCallback::findSampleAsset(const char* assetName, SampleFramework::SampleAsset::Type type)
{
	SampleFramework::SampleAsset* asset = 0;
#if WORK_AROUND_BROKEN_ASSET_PATHS
	assetName = mapHackyPath(assetName);
#endif
	asset = m_assetManager.getAsset(assetName, type);
	PX_ASSERT(asset);
	return asset;
}

#if WORK_AROUND_BROKEN_ASSET_PATHS

struct HackyPath
{
	const char* original;
	const char* mapped;
};

const char* SampleApexResourceCallback::mapHackyPath(const char* path)
{
	const char* mappedPath = path;
	static const HackyPath map[] =
	{
		// John, fix the apex debug renderer to allow the user to specify the material path...
		{ "ApexSolidShaded", "materials/simple_lit_color.xml"   },
		{ "ApexWireframe",   "materials/simple_unlit.xml"       },
	};
	const physx::PxU32 mapSize = PX_ARRAY_SIZE(map);
	for (physx::PxU32 i = 0; i < mapSize; i++)
	{
		const HackyPath& hp = map[i];
		if (!strcmp(hp.original, mappedPath))
		{
			mappedPath = hp.mapped;
			break;
		}
	}
	PX_ASSERT(mappedPath == path && "IF YOU HIT THIS ASSET IT MEANS A HACKY PATH WAS MAPPED, FIX YOUR F-ING ASSET FILES!!!");
	return mappedPath;
}

bool SampleApexResourceCallback::xmlFileExtension(const char* name)
{
	const char* ext = getFileExtension(name);

	if (ext && !strcmp(".xml", ext))
	{
		return true;
	}
	else
	{
		return false;
	}
}

// This returns all extensions (.xml.flz, .xml, .ini.old)
const char* SampleApexResourceCallback::getFileExtension(const char* name)
{
	const char* ext = 0;
	for (const char* c = name; *c; c++)
	{
		if (*c == '/' || *c == '\\')
		{
			ext = 0;
		}
		else if (*c == '.')
		{
			ext = c;
		}
	}

	return ext;
}
#endif //WORK_AROUND_BROKEN_ASSET_PATHS


void* SampleApexResourceCallback::requestResource(const char* nameSpace, const char* pname)
{
	void* resource = 0;

	bool incrementNumGets = true;

	PX_ASSERT(nameSpace && *nameSpace);
	PX_ASSERT(pname && *pname);

#if DEBUG_RESOURCE_REQUESTS
	fprintf(gDebugOutput, "new  - %s\n", pname);
#endif

#if WORK_AROUND_BROKEN_ASSET_PATHS
	// look for goofy "::" characters...
	const char* p = pname;
	while (*p && *p != ':')
	{
		p++;
	}
	if (*p == ':')
	{
		PX_ASSERT(!"Obsolete asset name format, fix your assets!");
		return NULL;
	}
#endif

	if (!strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
	{
		SampleFramework::SampleAsset* asset = findSampleAsset(pname, SampleFramework::SampleAsset::ASSET_MATERIAL);
		if (asset)
		{
			resource = asset;
		}
	}
	else if (!strcmp(nameSpace, APEX_CUSTOM_VB_NAME_SPACE))
	{
		// We currently don't support any custom vertex semantics in the samples,
		// so the resource will simply be a copy the name.  A real game engine
		// could return a pointer to whatever they want, or a member of an enum of
		// custom semantics.  Whatever resources are returned here will be provided
		// in the Render Resources API, in the array:
		//     void ** NxUserRenderVertexBufferDesc::customBuffersIdents

		size_t len = strlen(pname);
		char* n = new char[len + 1];
		physx::string::strcpy_s(n, len + 1, pname);
		resource = (void*)(n);
	}
	else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_NAME_SPACE))
	{
		PX_ASSERT(0 && "NRP seed failure for" APEX_COLLISION_GROUP_NAME_SPACE);
	}
	else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_MASK_NAME_SPACE))
	{
		PX_ASSERT(0 && "NRP seed failure for " APEX_COLLISION_GROUP_MASK_NAME_SPACE);
	}
	else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_128_NAME_SPACE))
	{
		// Note: When using effects that were authored in the ParticleEffectTool the
		// collision group names are defined by an artist/designer and it is up to the application
		// to translate that name into a viable set of collision group flags.  The algorithm for
		// how to translate these named fields to a set of bit fields is application specific.
		// Within the context of the sample framework, we must simply return 'some' valid result.
		// The previous behavior of this code is that it required that all named collision groups
		// be predefined ahead of time.  However, within the context of using assets authored by
		// the ParticleEffectTool this is on longer the case.  These values are considered 'globals'
		// within the context of the APEX SDK and do not get specific release calls performed on them.
		// For this reason the 'number of gets' counter should not be incredmented.
#if NX_SDK_VERSION_MAJOR == 3
		incrementNumGets = false;
		const char *equal = strchr(pname,'='); // if it uses the ASCII encoded filter bits format...
		if ( equal )
		{
			physx::PxFilterData	*fdata = (physx::PxFilterData *)m_FilterBits->getEncodedFilterData(pname);
			resource = fdata;
		}
		else
		{
			static physx::PxFilterData filterData;
			resource = &filterData;
			memset(resource,0xFF,sizeof(filterData));
		}
#else
		PX_ASSERT(0 && "NRP seed failure for " APEX_COLLISION_GROUP_128_NAME_SPACE);
#endif
	}
	else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_64_NAME_SPACE))
	{
		incrementNumGets = false;
		static physx::PxU64 collisionGroup;
		resource = &collisionGroup;
		memset(resource,0xFF,sizeof(collisionGroup));
//		PX_ASSERT(0 && "NRP seed failure for " APEX_COLLISION_GROUP_64_NAME_SPACE);
	}
	else if (!strcmp(nameSpace, APEX_PHYSICS_MATERIAL_NAME_SPACE))
	{
		PX_ASSERT(0 && "NRP seed failure for " APEX_PHYSICS_MATERIAL_NAME_SPACE);
	}
	else
	{
#if APEX_USE_PARTICLES
		if ( mModuleParticles )	// If we are using the Particles module
		{
			// See if this data for this resource was preloaded into the particles module.
			NxParameterized::Interface *iface = mModuleParticles->locateResource(pname,nameSpace);
			if ( iface )
			{
				NxParameterized::Interface *copyInterface=NULL;
				iface->clone(copyInterface);	// Create a copy of the parameterize data.
				PX_ASSERT(copyInterface);
				if ( copyInterface )
				{
					physx::apex::NxApexAsset *asset = m_apexSDK->createAsset(copyInterface,pname);	// Create the asset using this NxParameterized::Inteface data
					PX_ASSERT(asset);
					resource = asset;	// return this resource that we just created
					if ( asset == NULL )
					{
						// If it failed to create the asset; destroy the interface data.
						copyInterface->destroy();
					}
				}
			}
		}
#endif

		if ( resource == NULL )
		{
			physx::apex::NxApexAsset* asset = 0;

			if (
	#if APEX_USE_PARTICLES
				!strcmp(nameSpace, NX_APEX_EMITTER_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_GROUND_EMITTER_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_IMPACT_EMITTER_AUTHORING_TYPE_NAME) ||
	#if NX_SDK_VERSION_MAJOR == 2
				!strcmp(nameSpace, NX_FLUID_IOS_AUTHORING_TYPE_NAME) ||
	#elif NX_SDK_VERSION_MAJOR == 3
				!strcmp(nameSpace, NX_PARTICLE_IOS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_FORCEFIELD_AUTHORING_TYPE_NAME) ||
	#endif
				!strcmp(nameSpace, NX_IOFX_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_EXPLOSION_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_FIELD_BOUNDARY_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_BASIC_IOS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_JET_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_ATTRACTOR_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_NOISE_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_VORTEX_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_WIND_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_TURBULENCE_FS_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_VELOCITY_SOURCE_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_HEAT_SOURCE_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_SUBSTANCE_SOURCE_AUTHORING_TYPE_NAME) ||
	#endif // APEX_USE_PARTICLES

				!strcmp(nameSpace, NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_RENDER_MESH_AUTHORING_TYPE_NAME) ||
				!strcmp(nameSpace, NX_CLOTHING_AUTHORING_TYPE_NAME)
			)
			{
				// Assets that are using NxParameterized (and serialized outside of APEX)
				// currently have an XML extension.
				PX_ASSERT(pname);

				physx::PxFileBuf* stream = 0;

				const char* ext = getFileExtension(pname);
				if (ext)
				{
					stream = findApexAsset(pname);
				}
				else
				{
					if (XML_ASSET == m_assetPreference)
					{
						if (doesFileExist(pname, ".apx"))
						{
							ext = ".apx";
						}

						if (!ext && doesFileExist(pname, ".apb"))
						{
							ext = ".apb";
						}
					}
					else if (BIN_ASSET == m_assetPreference)
					{
						if (!ext && doesFileExist(pname, ".apb"))
						{
							ext = ".apb";
						}

						if (!ext && doesFileExist(pname, ".apx"))
						{
							ext = ".apx";
						}
					}
					else
					{
						// We prefer binary files in shipping builds

	#					ifdef APEX_SHIPPING
							if (!ext && doesFileExist(pname, ".apb"))
							{
								ext = ".apb";
							}
	#					endif

						if (!ext && doesFileExist(pname, ".apx"))
						{
							ext = ".apx";
						}

	#					ifndef APEX_SHIPPING
							if (!ext && doesFileExist(pname, ".apb"))
							{
								ext = ".apb";
							}
	#					endif
					}

					PX_ASSERT(ext);
					if (ext)
					{
						char fullname[512] = {0};
						physx::string::strcpy_s(fullname, sizeof(fullname), pname);
						physx::string::strcat_s(fullname, sizeof(fullname), ext);

						stream = findApexAsset(fullname);
					}
				}

				if (stream)
				{
					// we really shouldn't have extensions in our asset names, and apps should
					// determine the serialization type using this NxApesSDK::getSerializeType() method
					NxParameterized::Serializer::SerializeType serType = m_apexSDK->getSerializeType(*stream);

					if (ext)
					{
						NxParameterized::Serializer::SerializeType iSerType;
						if (0 == strcmp(".apx", ext))
						{
							iSerType = NxParameterized::Serializer::NST_XML;
						}
						else if (0 == strcmp(".apb", ext))
						{
							iSerType = NxParameterized::Serializer::NST_BINARY;
						}
						else
						{
							iSerType = NxParameterized::Serializer::NST_LAST;
							PX_ASSERT(0 && "Invalid asset file extension");
						}

						// PH: If you end up here, you have a binary file with an xml extension (.apx or .xml) or vice versa
						PX_ASSERT(iSerType == serType && "Wrong file extension??");
						PX_UNUSED(iSerType);
					}

					NxParameterized::Serializer::ErrorType serError;

					NxParameterized::SerializePlatform platform;
					serError = m_apexSDK->getSerializePlatform(*stream, platform);
					PX_ASSERT(serError == NxParameterized::Serializer::ERROR_NONE);

					NxParameterized::Serializer*  ser = m_apexSDK->createSerializer(serType);
					PX_ASSERT(ser);
					if (NULL==ser)
					{
						if (m_renderer.getErrorCallback())
						{
							// emit a "missing asset" warning using output error stream
							char msg[1024];

							physx::string::sprintf_s(msg, sizeof(msg), "Error creating the serializer for asset <%s> in namespace <%s>", pname, nameSpace);
							m_renderer.getErrorCallback()->reportError(physx::PxErrorCode::eDEBUG_WARNING, msg, __FILE__, __LINE__);
						}

						if (stream)
						{
							stream->release();
							stream = NULL;
						}
						return NULL;
					}

					NxParameterized::Serializer::DeserializedData data;
					NxParameterized::SerializePlatform currentPlatform;
					m_apexSDK->getCurrentPlatform(currentPlatform);
					NxParameterized::Traits* t = m_apexSDK->getParameterizedTraits();
					physx::PxU32 len = stream->getFileLength();

					if (NxParameterized::Serializer::NST_BINARY == serType && INPLACE_BINARY && platform == currentPlatform)
					{
						void* p = t->alloc(len);
						stream->read(p, len);
						serError = ser->deserializeInplace(p, len, data);
					}
					else if (NxParameterized::Serializer::NST_BINARY == serType)
					{
						// If the asset is binary but not inplace, read it into a memory buffer (MUCH faster with PS3 dev env).
						// We could do this with XML files as well, but if there's a huge XML asset the consoles may fail on
						// the allocation.
						void* p = t->alloc(len);
						stream->read(p, len);
						physx::PxFileBuf* memStream = m_apexSDK->createMemoryReadStream(p, len);

						serError = ser->deserialize(*memStream, data);

						m_apexSDK->releaseMemoryReadStream(*memStream);
						t->free(p);
					}
					else
					{
						serError = ser->deserialize(*stream, data);
					}

					if (serError == NxParameterized::Serializer::ERROR_NONE && data.size() == 1)
					{
						NxParameterized::Interface* params = data[0];
						asset = m_apexSDK->createAsset(params, pname);
						PX_ASSERT(asset && "ERROR Creating NxParameterized Asset");
					}
					else
					{
						PX_ASSERT(0 && "ERROR Deserializing NxParameterized Asset");
					}

					stream->release();
					ser->release();
				}
			}


			PX_ASSERT(asset);
			if (asset)
			{
				bool rightType = strcmp(nameSpace, asset->getObjTypeName()) == 0;
				PX_ASSERT(rightType);
				if (rightType)
				{
					resource = asset;
				}
				else
				{
					m_apexSDK->releaseAsset(*asset);
					asset = 0;
				}
			}
		}
	}

	if (resource )
	{
		if ( incrementNumGets )
		{
			m_numGets++;
		}
	}
	else if (m_renderer.getErrorCallback())
	{
		// emit a "missing asset" warning using output error stream
		char msg[1024];

		physx::string::sprintf_s(msg, sizeof(msg), "Could not find asset <%s> in namespace <%s>", pname, nameSpace);
		m_renderer.getErrorCallback()->reportError(physx::PxErrorCode::eDEBUG_WARNING, msg, __FILE__, __LINE__);
	}

	return resource;
}

void SampleApexResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{
	PX_ASSERT(resource);
	PX_UNUSED(name);
	if (resource)
	{

#if DEBUG_RESOURCE_REQUESTS
		fprintf(gDebugOutput, "free - %s\n", name);
#endif
		if (!strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
		{
			SampleFramework::SampleMaterialAsset* asset = static_cast<SampleFramework::SampleMaterialAsset*>(resource);
			m_assetManager.returnAsset(*asset);
		}
		else if (!strcmp(nameSpace, APEX_CUSTOM_VB_NAME_SPACE))
		{
			delete(char*) resource;	// char* allocated above with new
		}
		else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_128_NAME_SPACE))
		{
			PX_ALWAYS_ASSERT();
		}
		else if (!strcmp(nameSpace, APEX_COLLISION_GROUP_64_NAME_SPACE))
		{
			PX_ALWAYS_ASSERT();
		}
		else
		{
			physx::apex::NxApexAsset* asset = (physx::apex::NxApexAsset*)resource;
			m_apexSDK->releaseAsset(*asset);
		}
		m_numGets--;
	}
}
