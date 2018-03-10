/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef CONVERSIONCLOTHINGASSETPARAMETERS_0P7_0P8H_H
#define CONVERSIONCLOTHINGASSETPARAMETERS_0P7_0P8H_H

#include "ParamConversionTemplate.h"
#include "ClothingAssetParameters_0p7.h"
#include "ClothingAssetParameters_0p8.h"

#include "ClothingPhysicalMeshParameters_0p8.h"

namespace physx
{
namespace apex
{
namespace legacy
{

typedef ParamConversionTemplate<ClothingAssetParameters_0p7, ClothingAssetParameters_0p8, 7, 8> ConversionClothingAssetParameters_0p7_0p8Parent;

class ConversionClothingAssetParameters_0p7_0p8: ConversionClothingAssetParameters_0p7_0p8Parent
{
public:
	static NxParameterized::Conversion* Create(NxParameterized::Traits* t)
	{
		void* buf = t->alloc(sizeof(ConversionClothingAssetParameters_0p7_0p8));
		return buf ? PX_PLACEMENT_NEW(buf, ConversionClothingAssetParameters_0p7_0p8)(t) : 0;
	}

protected:
	ConversionClothingAssetParameters_0p7_0p8(NxParameterized::Traits* t) : ConversionClothingAssetParameters_0p7_0p8Parent(t) {}

	const NxParameterized::PrefVer* getPreferredVersions() const
	{
		static NxParameterized::PrefVer prefVers[] =
		{
			//TODO:
			//	Add your preferred versions for included references here.
			//	Entry format is
			//		{ (const char*)longName, (PxU32)preferredVersion }

			{ "physicalMeshes[]", 8 },
			{ 0, 0 } // Terminator (do not remove!)
		};

		//PX_UNUSED(prefVers[0]); // Make compiler happy

		return prefVers;
	}

	bool convert()
	{
		//TODO:
		//	Write custom conversion code here using mNewData and mLegacyData members.
		//
		//	Note that
		//		- mNewData has already been initialized with default values
		//		- same-named/same-typed members have already been copied
		//			from mLegacyData to mNewData
		//		- included references were moved to mNewData
		//			(and updated to preferred versions according to getPreferredVersions)
		//
		//	For more info see the versioning wiki.

#if 0
		// PH: It seems this was a bad idea. So now old assets will not have any virtual particles.

		PxF32 smallestTriangleArea = PX_MAX_F32;
		PxF32 largestTriangleArea = 0.0f;

		const PxI32 numPhysicalMeshes = mNewData->physicalMeshes.arraySizes[0];
		for (PxI32 i = 0; i < numPhysicalMeshes; i++)
		{
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMajorVersion() == 0);
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMinorVersion() == 8);

			ClothingPhysicalMeshParameters_0p8& physicalMesh = static_cast<ClothingPhysicalMeshParameters_0p8&>(*mNewData->physicalMeshes.buf[i]);

			const PxU32 numIndices = physicalMesh.physicalMesh.numIndices;
			//const PxU32 numVertices = physicalMesh.physicalMesh.numVertices;
			const PxU32* indices = physicalMesh.physicalMesh.indices.buf;
			const PxVec3* vertices = physicalMesh.physicalMesh.vertices.buf;

			for (PxU32 j = 0; j < numIndices; j += 3)
			{
				const PxVec3 edge1 = vertices[indices[j + 1]] - vertices[indices[j]];
				const PxVec3 edge2 = vertices[indices[j + 2]] - vertices[indices[j]];
				const PxF32 triangleArea = edge1.cross(edge2).magnitude();

				smallestTriangleArea = PxMin(smallestTriangleArea, triangleArea);
				largestTriangleArea = PxMax(largestTriangleArea, triangleArea);
			}
		}

		const PxF32 sphereArea = mNewData->simulation.thickness * mNewData->simulation.thickness * PxPi;
		const PxF32 coveredTriangleArea = 1.5f * sphereArea;

		PxF32 globalMinDensity = 0.0f;

		for (PxI32 i = 0; i < numPhysicalMeshes; i++)
		{
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMajorVersion() == 0);
			PX_ASSERT(mNewData->physicalMeshes.buf[i]->getMinorVersion() == 8);

			ClothingPhysicalMeshParameters_0p8& physicalMesh = static_cast<ClothingPhysicalMeshParameters_0p8&>(*mNewData->physicalMeshes.buf[i]);

			const PxU32 numIndices = physicalMesh.physicalMesh.numIndices;
			const PxU32* indices = physicalMesh.physicalMesh.indices.buf;
			const PxVec3* vertices = physicalMesh.physicalMesh.vertices.buf;

			for (PxU32 j = 0; j < numIndices; j += 3)
			{
				const PxVec3 edge1 = vertices[indices[j + 1]] - vertices[indices[j]];
				const PxVec3 edge2 = vertices[indices[j + 2]] - vertices[indices[j]];
				const PxF32 triangleArea = edge1.cross(edge2).magnitude();

				const PxF32 oldNumSpheres = (triangleArea - coveredTriangleArea) / sphereArea;
				if (oldNumSpheres > 0.8f)
				{
					const PxF32 reverseMinTriangleArea = triangleArea / oldNumSpheres;
					const PxF32 minDensity = PxClamp((reverseMinTriangleArea - largestTriangleArea) / (smallestTriangleArea / 2.0f - largestTriangleArea), 0.0f, 1.0f);

					globalMinDensity = PxMax(globalMinDensity, minDensity);
				}
			}
		}

		mNewData->simulation.virtualParticleDensity = globalMinDensity;
#endif

		return true;
	}
};

}
}
} // namespace physx::apex

#endif
