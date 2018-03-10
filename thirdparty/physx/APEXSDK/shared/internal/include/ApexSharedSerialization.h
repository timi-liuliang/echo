/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef __APEXSHAREDSERIALIZATION_H__
#define __APEXSHAREDSERIALIZATION_H__

#include "NxApex.h"
#include "ApexStream.h"
#include "ApexString.h"
#include "ApexSharedUtils.h"
#include "NiApexSDK.h"
#include "NxParameterized.h"

namespace physx
{
namespace apex
{

struct ApexStreamVersion
{
	enum Enum
	{
		First = 0,
		ConsolidatedSubmeshes = 1,
		TriangleFlags = 2,
		ConvexHull_VolumeStreamed = 3,
		AddedDestrucibleDeformation = 4,
		RemovedParticleSizeFromDestructibleParameters = 5,
		TriangleFlagsChangedToExtraDataIndex = 6,
		ConvexHull_EdgesStreamed = 7,
		AddedDestrucibleDebrisParamaters = 8,
		DestrucibleDebrisParamatersHaveLODRange = 9,
		UniversalNamedAssetHeader = 10,
		ClothMeshBindPoses = 10, // PH: This is not a mistake, this enum was added in another branch while other enums were added in //trunk
		VariableVertexData = 11,
		SoftBodyNormalsStreamed = 11, // PH: This is not a mistake, this enum was added in another branch while other enums were added in //trunk
		UsingApexMaterialLibrary_and_NoFlagsInNxRenderTriangle = 12,
		AddingTextureTypeInformationToVertexFormat = 13,
		SoftBodyNormalsInTetraLink = 14,
		ClothingVertexFlagsStreamed = 15,
		ConvexHullModifiedForEfficientCollisionTests = 16,
		RenderMeshAssetRedesign = 17,
		FixedSerializationAfterRenderMeshAssetRedesign = 18,
		AddedRenderCullModeToRenderMeshAsset = 19,
		AddedTextureUVOriginField = 19,	// BRG: merge conflict
		AddedDynamicVertexBufferField = 20,
		RenderMeshAssetBufferOverrunFix = 21,
		CleanupOfApexRenderMesh = 22,
		RemovedTextureTypeInformationFromVertexFormat = 23,
		AddedDestructibleFlag_FormExtendedStructures = 24,
		MarkingDestructibleChunksWithFlagToDenoteUnfracturableDescendant = 25,
		AddedValidBoundsToDestructibleParameters = 26,
		ReducingAssetUponDeserializationForReducedLOD = 27,
		AddedDustMeshParticleFactoryNameToDestructibleAsset = 28,
		AddedChunkSurfaceTracesToDestructibleAsset = 29,
		AddedInteriorSubmeshIndexToDestructibleAsset = 30,
		AddedMaxChunkSpeedAndMassScaleExponentToDestructibleAsset = 31,
		AddedGrbVolumeLimit = 32,
		Removing_rollSign_and_rollAxis_fromAPI = 33,
		AddedFractureImpulseScale = 34,
		AddedMissingRuntimeFlagsAndAlsoTraceSetAverageNormalToDestructibleAssetStream = 35,
		CachingChunkOverlapDataInDestructibleAsset = 36,
		AddedRotationCaps = 37,
		AddedImpactVelocityThreshold = 37, // SJB: Trust me, this should be 37
		ChangedVertexColorFormatToReal = 38,
		AddedEssentialDepthParameter = 39,
		AddedSeparateBoneBufferFlagToRenderMeshAsset = 40,
		DoNotNormalizeBoneWeightsOnLoad = 41, // PH: This is a change for deserialization only, we don't need to do additional processing when newer than this
		MovedChunkSurfaceNormalFromTraceSetToChunk = 41,
		FlattenedVertexBuffer = 42,
		RemovedSubmeshTriangleStruct = 43,
		DestructibleAssetRearrangementForParameterization = 44,
		AddedDestructibleMaterialStrength = 45,
		ExposedChunkNeighborPadding = 46,
		ChangedVertexFormat = 47,
		UnifiedCustomAndStandardSemantics = 48,
		AddedAdjacentPlanesToConvexHullStream = 49,
		//	Always add a new version just before this line

		Count,
		Current = Count - 1
	};
};

/*****************************************************************************/
//	Non-versioned data (e.g. basic types) may use the << and >> operators
//	defined in ApexStream.h.  This file contains (de)serialization for
//	versioned data.
//
//	Serialization version is written "upstream" by a versioned parent object.
//	To add the version number to the streamed data for an object, record this
//	change in the ApexStreamVersion enum, and add the line:
//		stream << (physx::PxU32)ApexStreamVersion_Current;
//	to the serialize function.
//
//  Serialization signature:
//		void serialize( physx::PxFileBuf& stream, const typename& object )
//
//	Deserialization signature:
//		void deserialize( physx::PxFileBuf& stream, physx::PxU32 version, typename& object )
/*****************************************************************************/


// Template foo
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxExplicitRenderTriangle& t);
PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxExplicitRenderTriangle& t);
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxVertexUV& uv);
PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxVertexUV& uv);
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxVertex& v);
PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxVertex& v);
PX_INLINE void serialize(physx::PxFileBuf& stream, const ApexSimpleString& s);
PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, ApexSimpleString& s);
PX_INLINE void serialize(physx::PxFileBuf& stream,
                         physx::PxU32 headerVersion,
                         const NxParameterized::Interface* param,
                         NxParameterized::Handle& childHandle);
PX_INLINE void serialize(physx::PxFileBuf& stream,
                         physx::PxU32 headerVersion,
                         const NxParameterized::Interface* param,
                         const char* memberName,
                         NxParameterized::Handle* structHandle = NULL);
PX_INLINE void deserialize(physx::PxFileBuf& stream,
                           physx::PxU32 headerVersion,
                           NxParameterized::Interface* param,
                           NxParameterized::Handle& childHandle);
PX_INLINE void deserialize(physx::PxFileBuf& stream,
                           physx::PxU32 headerVersion,
                           NxParameterized::Interface* param,
                           const char* memberName,
                           NxParameterized::Handle* structHandle = NULL);

// Utility to write an APEX asset stream header
PX_INLINE void serializeCurrentApexStreamVersion(physx::PxFileBuf& stream, const ApexSimpleString& assetTypeName, physx::PxU32 assetVersion)
{
	stream << (physx::PxU32)ApexStreamVersion::Current;
	serialize(stream, assetTypeName);
	stream << assetVersion;
}

// Utility to read version header from the start of an APEX asset stream
PX_INLINE ApexStreamVersion::Enum deserializeApexStreamVersion(physx::PxFileBuf& stream, const ApexSimpleString& assetTypeName, physx::PxU32& assetVersion)
{
	physx::PxU32 headerVersion;
	stream >> headerVersion;
	if (headerVersion > ApexStreamVersion::Current)
	{
		APEX_INTERNAL_ERROR("Stream version (%d) is newer than this library (%d)", headerVersion, ApexStreamVersion::Current);
		PX_ALWAYS_ASSERT();
	}
	else if (headerVersion >= ApexStreamVersion::UniversalNamedAssetHeader)
	{
		ApexSimpleString streamedName;
		deserialize(stream, headerVersion, streamedName);
		if (streamedName == assetTypeName)
		{
			stream >> assetVersion;
		}
		else
		{
			APEX_INTERNAL_ERROR("Asset type name mismatch.  File <%s> != asset name <%s>",
			                    assetTypeName.c_str(), streamedName.c_str());
			PX_ALWAYS_ASSERT();
		}
	}
	return (ApexStreamVersion::Enum)headerVersion;
}

// Utility to read version header from the start of an APEX asset stream, when the type is unknown
PX_INLINE ApexStreamVersion::Enum deserializeGenericApexStreamVersion(physx::PxFileBuf& stream, ApexSimpleString& outAssetTypeName, physx::PxU32& outAssetVersion)
{
	physx::PxU32 headerVersion;
	stream >> headerVersion;

	if (headerVersion > ApexStreamVersion::Current)
	{
		APEX_INTERNAL_ERROR("Stream version (%d) is newer than this library (%d)", headerVersion, ApexStreamVersion::Current);
		PX_ALWAYS_ASSERT();
	}
	else if (headerVersion >= ApexStreamVersion::UniversalNamedAssetHeader)
	{
		ApexSimpleString streamedName;
		deserialize(stream, headerVersion, outAssetTypeName);
		stream >> outAssetVersion;
	}
	else
	{
		APEX_INTERNAL_ERROR("Stream version (%d) does not contain a universal named asset header, " \
		                    "use a specific asset creation method to load this asset", headerVersion);
		PX_ALWAYS_ASSERT();
	}
	return (ApexStreamVersion::Enum)headerVersion;
}

// Versioned wrappers for non-versioned data
template <class T> PX_INLINE void serialize(physx::PxFileBuf& stream, const T& t)
{
	stream << t;
}

template <class T> PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, T& t)
{
	PX_UNUSED(version);
	stream >> t;
}


// physx::Array<T>
template <class T> PX_INLINE void serialize(physx::PxFileBuf& stream, const physx::Array<T>& array)
{
	const physx::PxU32 size = array.size();
	stream << size;
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		serialize(stream, array[i]);
	}
}

template <class T> PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, physx::Array<T>& array)
{
	physx::PxU32 size;
	stream >> size;
	array.resize(size);
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		deserialize(stream, version, array[i]);
	}
}

// Several serialized objects have multiple associated versions
template <class T> PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version0, physx::PxU32 version1, physx::Array<T>& array)
{
	physx::PxU32 size;
	stream >> size;
	array.resize(size);
	for (physx::PxU32 i = 0; i < size; ++i)
	{
		deserialize(stream, version0, version1, array[i]);
	}
}

// NxVertexUV
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxVertexUV& uv)
{
	stream << uv.u << uv.v;
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxVertexUV& uv)
{
	// original version
	PX_UNUSED(version);
	stream >> uv.u >> uv.v;
}


// NxVertexColor
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxVertexColor& c)
{
	stream << c.r << c.g << c.b << c.a;
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxVertexColor& c)
{
	// original version
	PX_UNUSED(version);
	stream >> c.r >> c.g >> c.b >> c.a;
}


// NxVertex
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxVertex& v)
{
	//
	stream << v.position << v.normal << v.tangent << v.binormal;
	serialize(stream, v.uv[0]);
	serialize(stream, v.uv[1]);
	serialize(stream, v.uv[2]);
	serialize(stream, v.uv[3]);
	serialize(stream, v.color);
	for (physx::PxU32 i = 0; i < 4; i++)
	{
		stream << v.boneIndices[i];
		stream << v.boneWeights[i];
	}
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxVertex& v)
{
	if (version < ApexStreamVersion::RenderMeshAssetRedesign)
	{
		// set all other indices and weights to 0
		v.boneWeights[0] = 1.0f;
		for (physx::PxU32 i = 1; i < 4; i++)
		{
			v.boneIndices[i] = 0;
			v.boneWeights[i] = 0.0f;
		}
	}

	if (version >= ApexStreamVersion::VariableVertexData)
	{
		stream >> v.position >> v.normal >> v.tangent >> v.binormal;
		deserialize(stream, version, v.uv[0]);
		deserialize(stream, version, v.uv[1]);
		deserialize(stream, version, v.uv[2]);
		deserialize(stream, version, v.uv[3]);
		if (version >= ApexStreamVersion::ChangedVertexColorFormatToReal)
		{
			deserialize(stream, version, v.color);
		}
		else
		{
			physx::PxU32 intColor;
			stream >> intColor;
			v.color = NxVertexColor(PxColorRGBA((PxU8)(intColor & 255), (PxU8)((intColor >> 8) & 255), (PxU8)((intColor >> 16) & 255), (PxU8)(intColor >> 24)));
		}
		if (version >= ApexStreamVersion::RenderMeshAssetRedesign)
		{
			for (physx::PxU32 i = 0; i < 4; i++)
			{
				stream >> v.boneIndices[i];
				stream >> v.boneWeights[i];
			}
			if (version < ApexStreamVersion::RenderMeshAssetBufferOverrunFix)
			{
				physx::PxU16 dummyU16;
				physx::PxF32 dummyF32;
				for (physx::PxU32 i = 0; i < 4; i++)
				{
					stream >> dummyU16;
					stream >> dummyF32;
				}
			}
		}
		else
		{
			physx::PxU32 boneIndex;
			stream >> boneIndex;
			v.boneIndices[0] = (physx::PxU16)boneIndex;
		}
	}
	else
	{
		stream >> v.position >> v.normal >> v.tangent >> v.binormal >> v.uv[0][0] >> v.uv[0][1];
		physx::PxU32 intColor;
		stream >> intColor;
		v.color = NxVertexColor(PxColorRGBA((PxU8)(intColor & 255), (PxU8)((intColor >> 8) & 255), (PxU8)((intColor >> 16) & 255), (PxU8)(intColor >> 24)));
		stream >> v.boneIndices[0];
	}
}


// NxExplicitRenderTriangle
PX_INLINE void serialize(physx::PxFileBuf& stream, const NxExplicitRenderTriangle& t)
{
	serialize(stream, t.vertices[0]);
	serialize(stream, t.vertices[1]);
	serialize(stream, t.vertices[2]);
	stream << t.submeshIndex << t.smoothingMask << t.extraDataIndex;
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, NxExplicitRenderTriangle& t)
{
	deserialize(stream, version, t.vertices[0]);
	deserialize(stream, version, t.vertices[1]);
	deserialize(stream, version, t.vertices[2]);
	stream >> t.submeshIndex >> t.smoothingMask;
	if (version >= ApexStreamVersion::TriangleFlagsChangedToExtraDataIndex)
	{
		stream >> t.extraDataIndex;
	}
	else
	{
		if (version >= ApexStreamVersion::TriangleFlags)
		{
			physx::PxU32 deadData;
			stream >> deadData;
		}
		t.extraDataIndex = 0xFFFFFFFF;
	}
}

// ApexSimpleString
PX_INLINE void serialize(physx::PxFileBuf& stream, const ApexSimpleString& s)
{
	s.serialize(stream);
	//stream << s.len();
	//stream.storeBuffer( s.c_str(), s.len() );
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, ApexSimpleString& s)
{
	PX_UNUSED(version);
	s.deserialize(stream);
}


// ConvexHull
PX_INLINE void serialize(physx::PxFileBuf& stream, const ConvexHull& h)
{
	if (h.mParams == NULL)
	{
		PX_ASSERT(!"Attempting to serialize a ConvexHull with NULL NxParameters.");
		return;
	}

	Array<PxVec3>	vertices;
	Array<physx::PxPlane>	uniquePlanes;
	Array<PxU32>	edges;
	Array<PxU32>	adjacentFaces;
	Array<PxF32>	widths;

	NxParameterized::Handle handle(*h.mParams);

	// vertices
	vertices.resize((physx::PxU32)h.mParams->vertices.arraySizes[0]);
	h.mParams->getParameterHandle("vertices", handle);
	h.mParams->getParamVec3Array(handle, vertices.begin(), (physx::PxI32)vertices.size());
	serialize(stream, vertices);

	// uniquePlanes
	uniquePlanes.resize((physx::PxU32)h.mParams->uniquePlanes.arraySizes[0]);
	for (physx::PxU32 i = 0; i < (physx::PxU32)h.mParams->uniquePlanes.arraySizes[0]; ++i)
	{
		ConvexHullParametersNS::Plane_Type& plane = h.mParams->uniquePlanes.buf[i];
		uniquePlanes[i] = physx::PxPlane(plane.normal, plane.d);
	}
	serialize(stream, uniquePlanes);

	// bounds
	stream << h.mParams->bounds;

	// volume
	stream << h.mParams->volume;

	// edges
	edges.resize((physx::PxU32)h.mParams->edges.arraySizes[0]);
	h.mParams->getParameterHandle("edges", handle);
	h.mParams->getParamU32Array(handle, edges.begin(), (physx::PxI32)edges.size());
	serialize(stream, edges);

	// adjacentFaces
	adjacentFaces.resize((physx::PxU32)h.mParams->adjacentFaces.arraySizes[0]);
	h.mParams->getParameterHandle("adjacentFaces", handle);
	h.mParams->getParamU32Array(handle, adjacentFaces.begin(), (physx::PxI32)adjacentFaces.size());
	serialize(stream, adjacentFaces);

	// widths
	widths.resize((physx::PxU32)h.mParams->widths.arraySizes[0]);
	h.mParams->getParameterHandle("widths", handle);
	h.mParams->getParamF32Array(handle, widths.begin(), (physx::PxI32)widths.size());
	serialize(stream, widths);

	// unique direction count
	stream << h.mParams->uniqueEdgeDirectionCount;

	// plane count
	stream << h.mParams->planeCount;
}

PX_INLINE void deserialize(physx::PxFileBuf& stream, physx::PxU32 version, ConvexHull& h)
{
	if (h.mParams == NULL)
	{
		h.init();
	}

	Array<PxVec3>	vertices;
	Array<physx::PxPlane>	uniquePlanes;
	Array<PxU32>	edges;
	Array<PxU32>	adjacentFaces;
	Array<PxF32>	widths;

	NxParameterized::Handle handle(*h.mParams);

	// vertices
	deserialize(stream, version, vertices);
	h.mParams->getParameterHandle("vertices", handle);
	h.mParams->resizeArray(handle, (physx::PxI32)vertices.size());
	h.mParams->setParamVec3Array(handle, vertices.begin(), (physx::PxI32)vertices.size());

	// uniquePlanes
	deserialize(stream, version, uniquePlanes);
	h.mParams->getParameterHandle("uniquePlanes", handle);
	h.mParams->resizeArray(handle, (physx::PxI32)uniquePlanes.size());
	for (physx::PxU32 i = 0; i < uniquePlanes.size(); ++i)
	{
		physx::PxPlane& plane = uniquePlanes[i];
		ConvexHullParametersNS::Plane_Type& paramPlane = h.mParams->uniquePlanes.buf[i];
		paramPlane.normal = plane.n;
		paramPlane.d = plane.d;
	}

	// bounds
	stream >> h.mParams->bounds;

	// volume
	if (version >= ApexStreamVersion::ConvexHull_VolumeStreamed)
	{
		stream >> h.mParams->volume;
	}

	bool needsRebuild = false;

	if (version >= ApexStreamVersion::ConvexHull_EdgesStreamed)
	{
		// edges
		deserialize(stream, version, edges);
		h.mParams->getParameterHandle("edges", handle);
		h.mParams->resizeArray(handle, (physx::PxI32)edges.size());
		h.mParams->setParamU32Array(handle, edges.begin(), (physx::PxI32)edges.size());

		if (version >= ApexStreamVersion::AddedAdjacentPlanesToConvexHullStream)
		{
			// adjacentFaces
			deserialize(stream, version, adjacentFaces);
			h.mParams->getParameterHandle("adjacentFaces", handle);
			h.mParams->resizeArray(handle, (physx::PxI32)adjacentFaces.size());
			h.mParams->setParamU32Array(handle, adjacentFaces.begin(), (physx::PxI32)adjacentFaces.size());
		}
		else
		{
			needsRebuild = true;
		}

		if (version >= ApexStreamVersion::ConvexHullModifiedForEfficientCollisionTests)
		{
			// widths
			deserialize(stream, version, widths);
			h.mParams->getParameterHandle("widths", handle);
			h.mParams->resizeArray(handle, (physx::PxI32)widths.size());
			h.mParams->setParamF32Array(handle, widths.begin(), (physx::PxI32)widths.size());

			// unique direction count
			stream >> h.mParams->uniqueEdgeDirectionCount;

			// plane count
			stream >> h.mParams->planeCount;
		}
		else
		{
			// Fix up
			// Ouch - rebuilding!
			// \todo - issue load performance warning
			h.buildFromPoints(vertices.begin(), vertices.size(), sizeof(physx::PxVec3));
		}
	}
	else
	{
		needsRebuild = true;
	}

	if (needsRebuild)
	{
		// Ouch - rebuilding!
		// \todo - issue load performance warning
		h.buildFromPoints(vertices.begin(), vertices.size(), sizeof(physx::PxVec3));
	}
}

// PxParamerized F32, U32, string, enum, and named ref
PX_INLINE void serialize(physx::PxFileBuf& stream,
                         physx::PxU32 headerVersion,
                         const NxParameterized::Interface* param,
                         NxParameterized::Handle& childHandle)
{
	PX_UNUSED(headerVersion);
	PX_UNUSED(param);
	PX_ASSERT(childHandle.getConstInterface() == param);

	ApexSimpleString tmpString;

	if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_REF)
	{
		// named reference (we should check that it is not INCLUDED)

		NxParameterized::Interface* childParamPtr = 0;

		// just in case it wasn't initialized
		//if( param->initParamRef(childHandle, NULL, true) != NxParameterized::ERROR_NONE )
		//{
		//	return;
		//}

		childHandle.getParamRef(childParamPtr);

		if (!childParamPtr)  //Special case hack...
		{
			childHandle.initParamRef(NULL, true);
			childHandle.getParamRef(childParamPtr);
		}

		if (childParamPtr)
		{
			tmpString = childParamPtr->className();
			tmpString.serialize(stream);

			tmpString = childParamPtr->name();
			tmpString.serialize(stream);
		}
	}
	else if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_STRING)
	{
		const char* str;
		childHandle.getParamString(str);
		tmpString = str;
		tmpString.serialize(stream);
	}
	else if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_ENUM)
	{
		const char* str;
		childHandle.getParamEnum(str);
		tmpString = str;
		tmpString.serialize(stream);
	}
	else
	{
		switch (childHandle.parameterDefinition()->type())
		{
		case NxParameterized::TYPE_VEC3:
		{
			physx::PxVec3 d;
			childHandle.getParamVec3(d);
			stream << d;
			break;
		}
		case NxParameterized::TYPE_F32:
		{
			physx::PxF32 d;
			childHandle.getParamF32(d);
			stream << d;
			break;
		}
		case NxParameterized::TYPE_U32:
		{
			physx::PxU32 d;
			childHandle.getParamU32(d);
			stream << d;
			break;
		}
		case NxParameterized::TYPE_BOOL:
		{
			bool d;
			childHandle.getParamBool(d);
			if (d)
			{
				stream.storeByte((physx::PxU8)1);
			}
			else
			{
				stream.storeByte((physx::PxU8)0);
			}
			break;
		}
		default:
			PX_ALWAYS_ASSERT();
		}
	}
}

PX_INLINE void serialize(physx::PxFileBuf& stream,
                         physx::PxU32 headerVersion,
                         const NxParameterized::Interface* param,
                         const char* memberName,
                         NxParameterized::Handle* structHandle)
{
	PX_UNUSED(headerVersion);

	NxParameterized::Handle childHandle(*param);
	if (structHandle)
	{
		structHandle->getChildHandle(param, memberName, childHandle);
	}
	else
	{
		param->getParameterHandle(memberName, childHandle);
	}

	serialize(stream, headerVersion, param, childHandle);
}


// PxParamerized F32, U32, string, enum, and named ref
// PxParamerized F32, U32, string, enum, and named ref
PX_INLINE void deserialize(physx::PxFileBuf& stream,
                           physx::PxU32 headerVersion,
                           NxParameterized::Interface* param,
                           NxParameterized::Handle& childHandle)
{
	PX_UNUSED(param);
	PX_ASSERT(childHandle.getConstInterface() == param);
	ApexSimpleString tmpString;

	if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_REF)
	{
		deserialize(stream, headerVersion, tmpString);

		// named reference (we should check that it is not INCLUDED
		NxParameterized::Interface* childParamPtr = 0;

		// tmpString is the className (asset authorable name)
		childHandle.initParamRef(tmpString.c_str(), true);
		childHandle.getParamRef(childParamPtr);

		PX_ASSERT(childParamPtr);
		if (childParamPtr)
		{
			deserialize(stream, headerVersion, tmpString);
			childParamPtr->setName(tmpString.c_str());
		}

	}
	else if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_STRING)
	{
		deserialize(stream, headerVersion, tmpString);
		childHandle.setParamString(tmpString.c_str());
	}
	else if (childHandle.parameterDefinition()->type() == NxParameterized::TYPE_ENUM)
	{
		deserialize(stream, headerVersion, tmpString);
		if (childHandle.setParamEnum(tmpString.c_str()) != NxParameterized::ERROR_NONE)
		{
			APEX_DEBUG_WARNING("NxParameterized ENUM value not correct: %s, substituting: %s", \
			                   tmpString.c_str(), childHandle.parameterDefinition()->enumVal(0));

			childHandle.setParamEnum(childHandle.parameterDefinition()->enumVal(0));
		}

	}
	else
	{
		switch (childHandle.parameterDefinition()->type())
		{
		case NxParameterized::TYPE_VEC3:
		{
			physx::PxVec3 d;
			stream >> d;
			childHandle.setParamVec3(d);
			break;
		}
		case NxParameterized::TYPE_F32:
		{
			physx::PxF32 d;
			stream >> d;
			childHandle.setParamF32(d);
			break;
		}
		case NxParameterized::TYPE_U32:
		{
			physx::PxU32 d;
			stream >> d;
			childHandle.setParamU32(d);
			break;
		}
		case NxParameterized::TYPE_BOOL:
		{
			bool d;
			physx::PxU8 value = stream.readByte();
			if (value)
			{
				d = true;
			}
			else
			{
				d = false;
			}

			childHandle.setParamBool(d);
			break;
		}
		default:
			PX_ALWAYS_ASSERT();
		}
	}

}

PX_INLINE void deserialize(physx::PxFileBuf& stream,
                           physx::PxU32 headerVersion,
                           NxParameterized::Interface* param,
                           const char* memberName,
                           NxParameterized::Handle* structHandle)
{
	NxParameterized::Handle childHandle(*param);
	if (structHandle)
	{
		structHandle->getChildHandle(param, memberName, childHandle);
	}
	else
	{
		param->getParameterHandle(memberName, childHandle);
	}

	deserialize(stream, headerVersion, param, childHandle);
}


}
} // end namespace physx::apex


#endif	// __APEXSHAREDSERIALIZATIONHELPERS_H__
