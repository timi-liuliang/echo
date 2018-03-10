/*=============================================================================
	LightingMesh.cpp: Static lighting mesh implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Importer.h"
#include "MonteCarlo.h"
#include "LightingSystem.h"

namespace Lightmass
{

/** 
 * Map from FStaticLightingMesh to the index given to uniquely identify all instances of the same primitive component.
 * This is used to give all LOD's of the same primitive component the same mesh index.
 */
TMap<FStaticLightingMesh*, INT> FStaticLightingMesh::MeshToIndexMap;

/** Evaluates the mesh's Bidirectional Reflectance Distribution Function. */
FLinearColor FStaticLightingMesh::EvaluateBRDF(
	const FStaticLightingVertex& Vertex, 
	INT ElementIndex,
	const FVector4& IncomingDirection, 
	const FVector4& OutgoingDirection) const
{
	checkSlow(Vertex.WorldTangentZ.IsUnit());
	checkSlow(IncomingDirection.IsUnit());
	checkSlow(OutgoingDirection.IsUnit());
	const FVector4 ReflectedIncomingVector = IncomingDirection.Reflect(Vertex.WorldTangentZ);
	const FLOAT OutgoingDotReflected = Max(OutgoingDirection | ReflectedIncomingVector, 0.0f);
	const FLinearColor Diffuse = EvaluateDiffuse(Vertex.TextureCoordinates[0], ElementIndex);
	const FLinearColor Specular = EvaluateSpecularAndPower(Vertex.TextureCoordinates[0], ElementIndex);
	const FLOAT SpecularPower = Specular.A;
	// Modified Phong BRDF from the Paper titled "Using the Modified Phong Reflectance Model for Physically Based Rendering"
	const FLOAT SpecularWeight = (SpecularPower + 2) / (2.0f * (FLOAT)PI);
	return Diffuse / (FLOAT)PI + Specular * SpecularWeight * appPow(OutgoingDotReflected, SpecularPower);
}

/** Generates an outgoing direction sample and evaluates the BRDF for that direction. */
FLinearColor FStaticLightingMesh::SampleBRDF(
	const FStaticLightingVertex& Vertex, 
	INT ElementIndex,
	const FVector4& IncomingDirection, 
	FVector4& OutgoingDirection,
	FLOAT& DirectionPDF,
	UBOOL& bSpecularSample,
	FRandomStream& RandomStream
	) const
{
	checkSlow(Vertex.WorldTangentZ.IsUnit());
	checkSlow(IncomingDirection.IsUnit());

	const FVector4 ReflectedIncomingVector = IncomingDirection.Reflect(Vertex.WorldTangentZ);
	const FVector4 TangentReflectedIncomingVector = Vertex.TransformWorldVectorToTangent(ReflectedIncomingVector);

	const FLinearColor Diffuse = EvaluateDiffuse(Vertex.TextureCoordinates[0], ElementIndex);
	const FLinearColor Specular = EvaluateSpecularAndPower(Vertex.TextureCoordinates[0], ElementIndex);
	const FLOAT SpecularPower = Specular.A;

	const FLOAT DiffuseIntensity = Diffuse.LinearRGBToXYZ().G;
	const FLOAT SepecularIntensity = Specular.LinearRGBToXYZ().G;
	// Randomly select diffuse or specular with a probability based on each term's contribution to the overall reflectance.
	//@todo - how does this probability affect the weighting of the generated direction?
	const FLOAT TermSelector = RandomStream.GetFraction() * (DiffuseIntensity + SepecularIntensity);

	FVector4 TangentPathDirection;
	if (TermSelector <= DiffuseIntensity)
	{
		// Generate a direction based on the cosine lobe
		TangentPathDirection = GetCosineHemisphereVector(RandomStream);

		const FLOAT CosTheta = Max(IncomingDirection | Vertex.WorldTangentZ, 0.0f);
		const FLOAT CosPDF = CosTheta / (FLOAT)PI;
		checkSlow(CosPDF > 0.0f);
		DirectionPDF = CosPDF;
		bSpecularSample = FALSE;
	}
	else
	{
		// Generate a direction based on the specular lobe
		TangentPathDirection = GetModifiedPhongSpecularVector(RandomStream, TangentReflectedIncomingVector, SpecularPower);

		const FLOAT CosAlpha = Max(TangentPathDirection | TangentReflectedIncomingVector, 0.0f);
		const FLOAT SpecularPDF = (SpecularPower + 1.0f) * appPow(CosAlpha, SpecularPower) / (2.0f * (FLOAT)PI);
		checkSlow(SpecularPDF > 0.0f);
		DirectionPDF = SpecularPDF;
		bSpecularSample = TRUE;
	}

	checkSlow(TangentPathDirection.Z >= 0.0f);
	checkSlow(TangentPathDirection.IsUnit());
	OutgoingDirection = Vertex.TransformTangentVectorToWorld(TangentPathDirection);
	checkSlow(OutgoingDirection.IsUnit());

	const FLOAT OutgoingDotReflected = Max(OutgoingDirection | ReflectedIncomingVector, 0.0f);
	// Modified Phong BRDF from the Paper titled "Using the Modified Phong Reflectance Model for Physically Based Rendering"
	const FLOAT SpecularWeight = (SpecularPower + 2) / (2.0f * (FLOAT)PI);
	FLinearColor BRDF = Diffuse / (FLOAT)PI + Specular * SpecularWeight * appPow(OutgoingDotReflected, SpecularPower);
	// So we can compare against FLinearColor::Black
	BRDF.A = 1.0f;
	return BRDF;
}

void FStaticLightingMesh::SetDebugMaterial(UBOOL bInUseDebugMaterial, FLinearColor InDiffuse, FLinearColor InSpecular, FLOAT InSpecularPower)
{
	bUseDebugMaterial = bInUseDebugMaterial;
	DebugDiffuse = InDiffuse;
	DebugSpecular = InSpecular;
	DebugSpecularPower = InSpecularPower;
}

void FStaticLightingMesh::Read( FILE* FileHandle, FScene& Scene)
{
	FStaticLightingMeshInstanceData TempData;
	fread( &TempData, sizeof(TempData), 1, FileHandle);
	Guid = TempData.Guid;
	NumTriangles = TempData.NumTriangles;
	NumShadingTriangles = TempData.NumShadingTriangles;
	NumVertices = TempData.NumVertices;
	NumShadingVertices = TempData.NumShadingVertices;
	MeshIndex = TempData.MeshIndex;
	LevelId = TempData.LevelId;
	TextureCoordinateIndex = TempData.TextureCoordinateIndex;
	LightingFlags = TempData.LightingFlags;
	bCastShadowAsTwoSided = TempData.bCastShadowAsTwoSided;
	bMovable = TempData.bMovable;
	NumRelevantLights = TempData.NumRelevantLights;
	bInstancedStaticMesh = TempData.bInstancedStaticMesh;
	BoundingBox = TempData.BoundingBox;

	// Read Relevant Lights
	INT NumRelevantLights = 0;
	fread( &NumRelevantLights, sizeof(INT), 1, FileHandle);
	for ( INT LightIndex=0; LightIndex<NumRelevantLights; LightIndex++)
	{
		FGuid LightGuid;
		fread( &LightGuid, sizeof(FGuid), 1, FileHandle);

		RelevantLights.AddItem( Scene.GetLights( LightGuid));
	}

	// Read VisibilityIds
	INT NumVisibilityIds = 0;
	INT VisibilityIDsVaule;
	fread( &NumVisibilityIds, sizeof(INT), 1, FileHandle);
	fread( &VisibilityIDsVaule, sizeof(INT), 1, FileHandle);
	VisibilityIds.AddItem( VisibilityIDsVaule);

	INT NumMaterialElements = 1;
	MaterialElements.Empty( NumMaterialElements);
	MaterialElements.Add( NumMaterialElements);
	for ( INT MtrIdx=0; MtrIdx<NumMaterialElements; MtrIdx++)
	{
		FMaterialElement& CurrentMaterialElement = MaterialElements(MtrIdx);
		FMaterialElementData TempData;
		fread( &TempData, sizeof(FMaterialElementData), 1, FileHandle);
		CurrentMaterialElement.MaterialId = TempData.MaterialId;
		CurrentMaterialElement.bUseTwoSidedLighting = TempData.bUseTwoSidedLighting;
		CurrentMaterialElement.bShadowIndirectOnly = TempData.bShadowIndirectOnly;
		CurrentMaterialElement.bUseEmissiveForStaticLighting = TempData.bUseEmissiveForStaticLighting;
		// Validating data here instead of in UE3 since EmissiveLightFalloffExponent is used in so many different object types
		CurrentMaterialElement.EmissiveLightFalloffExponent = Max(TempData.EmissiveLightFalloffExponent, 0.0f);
		CurrentMaterialElement.EmissiveLightExplicitInfluenceRadius = Max(TempData.EmissiveLightExplicitInfluenceRadius, 0.0f);
		CurrentMaterialElement.EmissiveBoost = TempData.EmissiveBoost;
		CurrentMaterialElement.DiffuseBoost = TempData.DiffuseBoost;
		CurrentMaterialElement.SpecularBoost = TempData.SpecularBoost;
		CurrentMaterialElement.FullyOccludedSamplesFraction = TempData.FullyOccludedSamplesFraction;
		CurrentMaterialElement.Material = Scene.GetMaterial( CurrentMaterialElement.MaterialId);
		checkf(CurrentMaterialElement.Material, TEXT("Failed to import material with GUID %s"), *CurrentMaterialElement.MaterialId.String());

		const UBOOL bMasked = CurrentMaterialElement.Material->BlendMode == BLEND_Masked || CurrentMaterialElement.Material->BlendMode == BLEND_SoftMasked;

		CurrentMaterialElement.bIsMasked = bMasked && CurrentMaterialElement.Material->TransmissionSize > 0;
		CurrentMaterialElement.bIsTwoSided = CurrentMaterialElement.Material->bTwoSided;
		CurrentMaterialElement.bTranslucent = !CurrentMaterialElement.bIsMasked && CurrentMaterialElement.Material->TransmissionSize > 0;
		CurrentMaterialElement.bCastShadowAsMasked = CurrentMaterialElement.Material->bCastShadowAsMasked;
	}
	bColorInvalidTexels = TRUE;
	bUseDebugMaterial = FALSE;
	DebugDiffuse = FLinearColor::Black;
	DebugSpecular = FLinearColor::Black;
	DebugSpecularPower = 1.0f;
}

void FStaticLightingMesh::Import( FLightmassImporter& Importer )
{
	// Import into a temporary struct and manually copy settings over,
	// Since the import will overwrite padding in FStaticLightingMeshInstanceData which is actual data in derived classes.
	FStaticLightingMeshInstanceData TempData;
	Importer.ImportData(&TempData);
	Guid = TempData.Guid;
	NumTriangles = TempData.NumTriangles;
	NumShadingTriangles = TempData.NumShadingTriangles;
	NumVertices = TempData.NumVertices;
	NumShadingVertices = TempData.NumShadingVertices;
	MeshIndex = TempData.MeshIndex;
	LevelId = TempData.LevelId;
	TextureCoordinateIndex = TempData.TextureCoordinateIndex;
	LightingFlags = TempData.LightingFlags;
	bCastShadowAsTwoSided = TempData.bCastShadowAsTwoSided;
	bMovable = TempData.bMovable;
	NumRelevantLights = TempData.NumRelevantLights;
	bInstancedStaticMesh = TempData.bInstancedStaticMesh;
	BoundingBox = TempData.BoundingBox;
	Importer.ImportGuidArray( RelevantLights, NumRelevantLights, Importer.GetLights() );

	INT NumVisibilityIds = 0;
	Importer.ImportData(&NumVisibilityIds);
	Importer.ImportArray(VisibilityIds, NumVisibilityIds);

	INT NumMaterialElements = 0;
	Importer.ImportData(&NumMaterialElements);
	check(NumMaterialElements > 0);
	MaterialElements.Empty(NumMaterialElements);
	MaterialElements.Add(NumMaterialElements);
	for (INT MtrlIdx = 0; MtrlIdx < NumMaterialElements; MtrlIdx++)
	{
		FMaterialElement& CurrentMaterialElement = MaterialElements(MtrlIdx);
		FMaterialElementData TempData;
		Importer.ImportData(&TempData);
		CurrentMaterialElement.MaterialId = TempData.MaterialId;
		CurrentMaterialElement.bUseTwoSidedLighting = TempData.bUseTwoSidedLighting;
		CurrentMaterialElement.bShadowIndirectOnly = TempData.bShadowIndirectOnly;
		CurrentMaterialElement.bUseEmissiveForStaticLighting = TempData.bUseEmissiveForStaticLighting;
		// Validating data here instead of in UE3 since EmissiveLightFalloffExponent is used in so many different object types
		CurrentMaterialElement.EmissiveLightFalloffExponent = Max(TempData.EmissiveLightFalloffExponent, 0.0f);
		CurrentMaterialElement.EmissiveLightExplicitInfluenceRadius = Max(TempData.EmissiveLightExplicitInfluenceRadius, 0.0f);
		CurrentMaterialElement.EmissiveBoost = TempData.EmissiveBoost;
		CurrentMaterialElement.DiffuseBoost = TempData.DiffuseBoost;
		CurrentMaterialElement.SpecularBoost = TempData.SpecularBoost;
		CurrentMaterialElement.FullyOccludedSamplesFraction = TempData.FullyOccludedSamplesFraction;
		CurrentMaterialElement.Material = Importer.ConditionalImportObject<FMaterial>(CurrentMaterialElement.MaterialId, LM_MATERIAL_VERSION, LM_MATERIAL_EXTENSION, LM_MATERIAL_CHANNEL_FLAGS, Importer.GetMaterials());
		checkf(CurrentMaterialElement.Material, TEXT("Failed to import material with GUID %s"), *CurrentMaterialElement.MaterialId.String());

		const UBOOL bMasked = CurrentMaterialElement.Material->BlendMode == BLEND_Masked || CurrentMaterialElement.Material->BlendMode == BLEND_SoftMasked;

		CurrentMaterialElement.bIsMasked = bMasked && CurrentMaterialElement.Material->TransmissionSize > 0;
		CurrentMaterialElement.bIsTwoSided = CurrentMaterialElement.Material->bTwoSided;
		CurrentMaterialElement.bTranslucent = !CurrentMaterialElement.bIsMasked && CurrentMaterialElement.Material->TransmissionSize > 0;
		CurrentMaterialElement.bCastShadowAsMasked = CurrentMaterialElement.Material->bCastShadowAsMasked;
	}
	bColorInvalidTexels = TRUE;
	bUseDebugMaterial = FALSE;
	DebugDiffuse = FLinearColor::Black;
	DebugSpecular = FLinearColor::Black;
	DebugSpecularPower = 1.0f;
}

/** Determines whether two triangles overlap each other's AABB's. */
static UBOOL AxisAlignedTriangleIntersectTriangle2d(
	const FVector2D& V0, const FVector2D& V1, const FVector2D& V2, 
	const FVector2D& OtherV0, const FVector2D& OtherV1, const FVector2D& OtherV2)
{
	const FVector2D MinFirst = Min(V0, Min(V1, V2));
	const FVector2D MaxFirst = Max(V0, Max(V1, V2));
	const FVector2D MinSecond = Min(OtherV0, Min(OtherV1, OtherV2));
	const FVector2D MaxSecond = Max(OtherV0, Max(OtherV1, OtherV2));

	return !(MinFirst.X > MaxSecond.X 
		|| MinSecond.X > MaxFirst.X 
		|| MinFirst.Y > MaxSecond.Y 
		|| MinSecond.Y > MaxFirst.Y);
}

static const INT UnprocessedIndex = -1;
static const INT PendingProcessingIndex = -2;
static const INT NotEmissiveIndex = -3;

/** Allows the mesh to create mesh area lights from its emissive contribution */
void FStaticLightingMesh::CreateMeshAreaLights(
	const FStaticLightingSystem& LightingSystem, 
	const FScene& Scene,
	TIndirectArray<FMeshAreaLight>& MeshAreaLights) const
{
	UBOOL bAnyElementsUseEmissiveForLighting = FALSE;
	for (INT MaterialIndex = 0; MaterialIndex < MaterialElements.Num(); MaterialIndex++)
	{
		if (MaterialElements(MaterialIndex).bUseEmissiveForStaticLighting &&
			MaterialElements(MaterialIndex).Material->EmissiveSize > 0)
		{
			bAnyElementsUseEmissiveForLighting = TRUE;
			break;
		}
	}

	if (!bAnyElementsUseEmissiveForLighting)
	{
		// Exit if none of the mesh's elements use emissive for lighting
		return;
	}

	// Emit warnings for meshes with lots of triangles, since the mesh area light creation is O(N^2) on the number of triangles
	if (NumTriangles > 3000 && NumTriangles <= 5000)
	{
		GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_WARNING, Guid, SOURCEOBJECTTYPE_Mapping, TEXT("LightmassError_EmissiveMeshHighPolyCount"));
	}
	else if (NumTriangles > 5000)
	{
		GSwarm->SendAlertMessage(NSwarm::ALERT_LEVEL_ERROR, Guid, SOURCEOBJECTTYPE_Mapping, TEXT("LightmassError_EmissiveMeshExtremelyHighPolyCount"));
		// This mesh will take a very long time to create mesh area lights for, so skip it
		return;
	}
	
	TArray<FStaticLightingVertex> MeshVertices;
	MeshVertices.Empty(NumTriangles * 3);
	MeshVertices.Add(NumTriangles * 3);

	TArray<INT> ElementIndices;
	ElementIndices.Empty(NumTriangles);
	ElementIndices.Add(NumTriangles);

	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		// Query the mesh for the triangle's vertices.
		GetTriangle(TriangleIndex, MeshVertices(TriangleIndex * 3 + 0), MeshVertices(TriangleIndex * 3 + 1), MeshVertices(TriangleIndex * 3 + 2), ElementIndices(TriangleIndex));
	}

	TArray<TArray<INT> > LayeredGroupTriangles;
	// Split the mesh into layers whose UVs do not overlap, maintaining adjacency in world space position and UVs.
	// This way meshes with tiling emissive textures are handled correctly, all instances of the emissive texels will emit light.
	CalculateUniqueLayers(MeshVertices, ElementIndices, LayeredGroupTriangles);

	// get Min/MaxUV on the mesh for the triangles
	FVector2D MinUV(FLT_MAX, FLT_MAX), MaxUV(-FLT_MAX, -FLT_MAX);
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		for (INT VIndex = 0; VIndex < 3; VIndex++)
		{
			const FStaticLightingVertex& CurrentVertex = MeshVertices(TriangleIndex * 3 + VIndex);
			MinUV.X = Min(MinUV.X, CurrentVertex.TextureCoordinates[TextureCoordinateIndex].X);
			MaxUV.X = Max(MaxUV.X, CurrentVertex.TextureCoordinates[TextureCoordinateIndex].X);
			MinUV.Y = Min(MinUV.Y, CurrentVertex.TextureCoordinates[TextureCoordinateIndex].Y);
			MaxUV.Y = Max(MaxUV.Y, CurrentVertex.TextureCoordinates[TextureCoordinateIndex].Y);
		}
	}

	// figure out many iterations of the texture we need (enough integer repetitions to cover the entire UV range used)
	// we floor the min and max because we need to see which integer wrap of UVs it falls into
	// so, if we had range .2 to .8, the floors would both go to 0, then add 1 to account for that one
	// if we have range -.2 to .3, we need space for the -1 .. 0 wrap, and the 0 to 1 wrap (ie 2 iterations)
	// @lmtodo: Actually, if the Min was used to modify the UV when looping through the Corners array,
	// we wouldn't need full integer ranges
	const INT NumIterationsX = (appFloor(MaxUV.X) - appFloor(MinUV.X)) + 1;
	const INT NumIterationsY = (appFloor(MaxUV.Y) - appFloor(MinUV.Y)) + 1;

	// calculate the bias and scale needed to map the random UV range into 0 .. NumIterations when rasterizing
	// into the TexelToCornersMap
	const FVector2D UVBias(-appFloor(MinUV.X), -appFloor(MinUV.Y));
	const FVector2D UVScale(1.0f / NumIterationsX, 1.0f / NumIterationsY);

	for (INT MaterialIndex = 0; MaterialIndex < MaterialElements.Num(); MaterialIndex++)
	{
		const FMaterial& CurrentMaterial = *MaterialElements(MaterialIndex).Material;
		if (MaterialElements(MaterialIndex).bUseEmissiveForStaticLighting &&
			CurrentMaterial.EmissiveSize > 0)
		{
			// Operate on each layer independently
			for (INT GroupIndex = 0; GroupIndex < LayeredGroupTriangles.Num(); GroupIndex++)
			{
				// Allocate a map from texel to the corners of that texel, giving enough space for all of the possible integer wraps
				FTexelToCornersMap TexelToCornersMap(NumIterationsX * CurrentMaterial.EmissiveSize, NumIterationsY * CurrentMaterial.EmissiveSize);
				LightingSystem.CalculateTexelCorners(LayeredGroupTriangles(GroupIndex), MeshVertices, TexelToCornersMap, ElementIndices, MaterialIndex, TextureCoordinateIndex, FALSE, UVBias, UVScale);

				for (INT Y = 0; Y < TexelToCornersMap.GetSizeY(); Y++)
				{
					for (INT X = 0; X < TexelToCornersMap.GetSizeX(); X++)
					{
						FTexelToCorners& CurrentTexelCorners = TexelToCornersMap(X, Y);
						// Normals need to be unit as their dot product will be used in comparisons later
						CurrentTexelCorners.WorldTangentZ = CurrentTexelCorners.WorldTangentZ.SizeSquared() > DELTA ? CurrentTexelCorners.WorldTangentZ.UnsafeNormal() : FVector4(0,0,1);
					}
				}

				TArray<INT> LightIndices;
				// Allocate an array of light indices, one for each texel, indexed by Y * SizeX + X
				LightIndices.Add(TexelToCornersMap.GetSizeX() * TexelToCornersMap.GetSizeY());
				// Initialize light indices to unprocessed
				appMemset(LightIndices.GetData(), UnprocessedIndex, LightIndices.Num() * LightIndices.GetTypeSize());
				INT NextLightIndex = 0;
				// The temporary stack of texels that need to be processed
				TArray<FIntPoint> TexelsInCurrentLight;
				// Iterate over all texels and assign a light index to each one
				for (INT Y = 0; Y < TexelToCornersMap.GetSizeY(); Y++)
				{
					for (INT X = 0; X < TexelToCornersMap.GetSizeX(); X++)
					{
						// Push the current texel onto the stack if it is emissive and hasn't been processed yet
						AddLightTexel(TexelToCornersMap, MaterialIndex, LightIndices, X, Y, Scene.MeshAreaLightSettings.EmissiveIntensityThreshold, TexelsInCurrentLight, CurrentMaterial.EmissiveSize, CurrentMaterial.EmissiveSize);
						if (TexelsInCurrentLight.Num() > 0)
						{
							// This is the first texel in a new light group
							const INT CurrentLightIndex = NextLightIndex;
							// Update the next light index
							NextLightIndex++;
							// Flood fill neighboring emissive texels with CurrentLightIndex
							// This is done with a temporary stack instead of recursion since the recursion depth can be very deep and overflow the stack
							while (TexelsInCurrentLight.Num() > 0)
							{
								// Remove the last texel in the stack
								const FIntPoint NextTexel = TexelsInCurrentLight.Pop();
								// Mark it as belonging to the current light
								LightIndices(NextTexel.Y * TexelToCornersMap.GetSizeX() + NextTexel.X) = CurrentLightIndex;
								// Push all of the texel's emissive, unprocessed neighbors onto the stack
								AddLightTexel(TexelToCornersMap, MaterialIndex, LightIndices, NextTexel.X - 1, NextTexel.Y, Scene.MeshAreaLightSettings.EmissiveIntensityThreshold, TexelsInCurrentLight, CurrentMaterial.EmissiveSize, CurrentMaterial.EmissiveSize);
								AddLightTexel(TexelToCornersMap, MaterialIndex, LightIndices, NextTexel.X + 1, NextTexel.Y, Scene.MeshAreaLightSettings.EmissiveIntensityThreshold, TexelsInCurrentLight, CurrentMaterial.EmissiveSize, CurrentMaterial.EmissiveSize);
								AddLightTexel(TexelToCornersMap, MaterialIndex, LightIndices, NextTexel.X, NextTexel.Y - 1, Scene.MeshAreaLightSettings.EmissiveIntensityThreshold, TexelsInCurrentLight, CurrentMaterial.EmissiveSize, CurrentMaterial.EmissiveSize);
								AddLightTexel(TexelToCornersMap, MaterialIndex, LightIndices, NextTexel.X, NextTexel.Y + 1, Scene.MeshAreaLightSettings.EmissiveIntensityThreshold, TexelsInCurrentLight, CurrentMaterial.EmissiveSize, CurrentMaterial.EmissiveSize);
							}
						}
					}
				}

				TArray<INT> PrimitiveIndices;
				PrimitiveIndices.Add(TexelToCornersMap.GetSizeX() * TexelToCornersMap.GetSizeY());
				appMemset(PrimitiveIndices.GetData(), UnprocessedIndex, PrimitiveIndices.Num() * PrimitiveIndices.GetTypeSize());
				INT NextPrimitiveIndex = 0;
				const FLOAT DistanceThreshold = FBoxSphereBounds(BoundingBox).SphereRadius * Scene.MeshAreaLightSettings.MeshAreaLightSimplifyMeshBoundingRadiusFractionThreshold;
				// The temporary stack of texels that need to be processed
				TArray<FIntPoint> PendingTexels;
				// Iterate over all texels and assign a primitive index to each one
				// This effectively simplifies the mesh area light by reducing the number of primitives that are needed to represent the light
				for (INT Y = 0; Y < TexelToCornersMap.GetSizeY(); Y++)
				{
					for (INT X = 0; X < TexelToCornersMap.GetSizeX(); X++)
					{
						const INT LightIndex = LightIndices(Y * TexelToCornersMap.GetSizeX() + X);
						// Every texel should have a valid light index or be marked not emissive by this pass
						checkSlow(LightIndex != UnprocessedIndex);
						checkSlow(LightIndex != PendingProcessingIndex);
						const FTexelToCorners& CurrentTexelCorners = TexelToCornersMap(X, Y);

						FVector4 PrimitiveCenter(0,0,0);
						for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
						{
							PrimitiveCenter += CurrentTexelCorners.Corners[CornerIndex].WorldPosition / (FLOAT)NumTexelCorners;
						}

						// Push the current texel onto the stack if it can be merged into the same primitive and hasn't been processed yet
						AddPrimitiveTexel(TexelToCornersMap, CurrentTexelCorners, LightIndex, PrimitiveCenter, PrimitiveIndices, LightIndices, X, Y, PendingTexels, Scene, DistanceThreshold);

						if (PendingTexels.Num() > 0) 
						{
							const INT CurrentPrimitiveIndex = NextPrimitiveIndex;
							// This is the first texel to go into a new primitive
							NextPrimitiveIndex++;
							// Flood fill neighboring texels with CurrentPrimitiveIndex
							// This is done with a temporary stack instead of recursion since the recursion depth can be very deep and overflow the stack
							while (PendingTexels.Num() > 0)
							{
								// Remove the last texel in the stack
								const FIntPoint NextTexel = PendingTexels.Pop();
								// Mark it as belonging to the current primitive
								PrimitiveIndices(NextTexel.Y * TexelToCornersMap.GetSizeX() + NextTexel.X) = CurrentPrimitiveIndex;
								const FTexelToCorners& NextTexelCorners = TexelToCornersMap(NextTexel.X, NextTexel.Y);
								const INT NextTexelLightIndex = LightIndices(NextTexel.Y * TexelToCornersMap.GetSizeX() + NextTexel.X);
								// Push all of the texel's neighbors onto the stack if they should be merged into the current primitive
								AddPrimitiveTexel(TexelToCornersMap, NextTexelCorners, NextTexelLightIndex, PrimitiveCenter, PrimitiveIndices, LightIndices, NextTexel.X - 1, NextTexel.Y, PendingTexels, Scene, DistanceThreshold);
								AddPrimitiveTexel(TexelToCornersMap, NextTexelCorners, NextTexelLightIndex, PrimitiveCenter, PrimitiveIndices, LightIndices, NextTexel.X + 1, NextTexel.Y, PendingTexels, Scene, DistanceThreshold);
								AddPrimitiveTexel(TexelToCornersMap, NextTexelCorners, NextTexelLightIndex, PrimitiveCenter, PrimitiveIndices, LightIndices, NextTexel.X, NextTexel.Y - 1, PendingTexels, Scene, DistanceThreshold);
								AddPrimitiveTexel(TexelToCornersMap, NextTexelCorners, NextTexelLightIndex, PrimitiveCenter, PrimitiveIndices, LightIndices, NextTexel.X, NextTexel.Y + 1, PendingTexels, Scene, DistanceThreshold);
							}
						}
					}
				}

				// An array of mesh light primitives for each light
				TArray<TArray<FMeshLightPrimitive>> EmissivePrimitives;
				// Allocate for the number of light indices that were assigned
				EmissivePrimitives.Empty(NextLightIndex);
				EmissivePrimitives.AddZeroed(NextLightIndex);
				for (INT LightIndex = 0; LightIndex < NextLightIndex; LightIndex++)
				{
					// Allocate for the number of primitives that were assigned
					EmissivePrimitives(LightIndex).Empty(NextPrimitiveIndex);
					EmissivePrimitives(LightIndex).AddZeroed(NextPrimitiveIndex);
				}

				for (INT Y = 0; Y < TexelToCornersMap.GetSizeY(); Y++)
				{
					const FLOAT YFraction = Y / (FLOAT)CurrentMaterial.EmissiveSize;
					for (INT X = 0; X < TexelToCornersMap.GetSizeX(); X++)
					{
						const INT LightIndex = LightIndices(Y * TexelToCornersMap.GetSizeX() + X);
						// Every texel should have a valid light index or be marked not emissive by this pass
						checkSlow(LightIndex != UnprocessedIndex);
						checkSlow(LightIndex != PendingProcessingIndex);
						if (LightIndex >= 0)
						{
							const FTexelToCorners& CurrentTexelCorners = TexelToCornersMap(X, Y);
							const INT PrimitiveIndex = PrimitiveIndices(Y * TexelToCornersMap.GetSizeX() + X);
							// Every texel should have a valid primitive index or be marked not emissive by this pass
							checkSlow(PrimitiveIndex != UnprocessedIndex);
							checkSlow(PrimitiveIndex != PendingProcessingIndex);
							if (PrimitiveIndex >= 0)
							{
								// Calculate the texel's center
								FVector4 TexelCenter(0,0,0);
								UBOOL bAllCornersValid = TRUE;
								for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
								{
									TexelCenter += CurrentTexelCorners.Corners[CornerIndex].WorldPosition / (FLOAT)NumTexelCorners;
									bAllCornersValid = bAllCornersValid && CurrentTexelCorners.bValid[CornerIndex];
								}
								checkSlow(bAllCornersValid);

								// Calculate the texel's bounding radius
								FLOAT TexelBoundingRadiusSquared = 0.0f;
								for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
								{
									const FLOAT CurrentRadiusSquared = (TexelCenter - CurrentTexelCorners.Corners[CornerIndex].WorldPosition).SizeSquared();
									if (CurrentRadiusSquared > TexelBoundingRadiusSquared)
									{
										TexelBoundingRadiusSquared = CurrentRadiusSquared;
									}
								}

								const FLOAT XFraction = X / (FLOAT)CurrentMaterial.EmissiveSize;
								const FLinearColor CurrentEmissive = EvaluateEmissive(FVector2D(XFraction, YFraction), MaterialIndex);
								checkSlow(CurrentEmissive.R > Scene.MeshAreaLightSettings.EmissiveIntensityThreshold 
									|| CurrentEmissive.G > Scene.MeshAreaLightSettings.EmissiveIntensityThreshold 
									|| CurrentEmissive.B > Scene.MeshAreaLightSettings.EmissiveIntensityThreshold);

								// Add a new primitive representing this texel to the light the texel was assigned to in the first pass
								EmissivePrimitives(LightIndex)(PrimitiveIndex).AddSubPrimitive(
									CurrentTexelCorners, 
									FIntPoint(X, Y), 
									CurrentEmissive, 
									// Offset the light primitives by a fraction of the texel's bounding radius to avoid incorrect self-occlusion,
									// Since the surface of the light is actually a mesh
									appSqrt(TexelBoundingRadiusSquared) * Scene.SceneConstants.VisibilityNormalOffsetSampleRadiusScale);
							}
						}
					}
				}

				TArray<TArray<FMeshLightPrimitive>> TrimmedEmissivePrimitives;
				TrimmedEmissivePrimitives.Empty(EmissivePrimitives.Num());
				TrimmedEmissivePrimitives.AddZeroed(EmissivePrimitives.Num());
				for (INT LightIndex = 0; LightIndex < EmissivePrimitives.Num(); LightIndex++)
				{
					TrimmedEmissivePrimitives(LightIndex).Empty(EmissivePrimitives(LightIndex).Num());
					for (INT PrimitiveIndex = 0; PrimitiveIndex < EmissivePrimitives(LightIndex).Num(); PrimitiveIndex++)
					{
						if (EmissivePrimitives(LightIndex)(PrimitiveIndex).NumSubPrimitives > 0)
						{
							// Only copy over primitives containing one or more sub primitives
							TrimmedEmissivePrimitives(LightIndex).AddItem(EmissivePrimitives(LightIndex)(PrimitiveIndex));
							TrimmedEmissivePrimitives(LightIndex).Last().Finalize();

							if (Scene.MeshAreaLightSettings.bVisualizeMeshAreaLightPrimitives)
							{
								// Draw 4 lines between the primitive corners for debugging
								// Currently hijacking ShadowRays
								LightingSystem.DebugOutput.ShadowRays.AddItem(FDebugStaticLightingRay(EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[0].WorldPosition - FVector4(0,0,.1f), EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[1].WorldPosition - FVector4(0,0,.1f), TRUE, FALSE));
								LightingSystem.DebugOutput.ShadowRays.AddItem(FDebugStaticLightingRay(EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[1].WorldPosition - FVector4(0,0,.1f), EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[3].WorldPosition - FVector4(0,0,.1f), TRUE, TRUE));
								LightingSystem.DebugOutput.ShadowRays.AddItem(FDebugStaticLightingRay(EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[3].WorldPosition - FVector4(0,0,.1f), EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[2].WorldPosition - FVector4(0,0,.1f), TRUE, FALSE));
								LightingSystem.DebugOutput.ShadowRays.AddItem(FDebugStaticLightingRay(EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[2].WorldPosition - FVector4(0,0,.1f), EmissivePrimitives(LightIndex)(PrimitiveIndex).Corners[0].WorldPosition - FVector4(0,0,.1f), TRUE, TRUE));
							}
						}
					}
				}

				// Create mesh area lights from each group of primitives that were gathered
				for (INT LightIndex = 0; LightIndex < TrimmedEmissivePrimitives.Num(); LightIndex++)
				{
					if (TrimmedEmissivePrimitives(LightIndex).Num() > 0)
					{
						// Initialize all of the mesh area light's unused properties to 0
						FMeshAreaLight* NewLight = new FMeshAreaLight(E_Init);
						NewLight->LightFlags = GI_LIGHT_HASSTATICLIGHTING | GI_LIGHT_USEDIRECTLIGHTMAP | GI_LIGHT_CASTSHADOWS | GI_LIGHT_CASTSTATICSHADOWS;
						NewLight->SetPrimitives(
							TrimmedEmissivePrimitives(LightIndex), 
							MaterialElements(MaterialIndex).EmissiveLightFalloffExponent, 
							MaterialElements(MaterialIndex).EmissiveLightExplicitInfluenceRadius,
							Scene.MeshAreaLightSettings.MeshAreaLightGridSize,
							LevelId);
						MeshAreaLights.AddItem(NewLight);
					}
				}
			}
		}
	}
}

/** Splits a mesh into layers with non-overlapping UVs, maintaining adjacency in world space and UVs. */
void FStaticLightingMesh::CalculateUniqueLayers(
	const TArray<FStaticLightingVertex>& MeshVertices, 
	const TArray<INT>& ElementIndices, 
	TArray<TArray<INT> >& LayeredGroupTriangles) const
{
	// Indices of adjacent triangles in world space, 3 indices for each triangle
	TArray<INT> WorldSpaceAdjacentTriangles;
	WorldSpaceAdjacentTriangles.Empty(NumTriangles * 3);
	WorldSpaceAdjacentTriangles.Add(NumTriangles * 3);
	// Adjacency for the mesh's triangles compared in texture space
	TArray<INT> TextureSpaceAdjacentTriangles;
	TextureSpaceAdjacentTriangles.Empty(NumTriangles * 3);
	TextureSpaceAdjacentTriangles.Add(NumTriangles * 3);

	// Initialize all triangles to having no adjacent triangles
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		WorldSpaceAdjacentTriangles(TriangleIndex * 3 + 0) = INDEX_NONE;
		WorldSpaceAdjacentTriangles(TriangleIndex * 3 + 1) = INDEX_NONE;
		WorldSpaceAdjacentTriangles(TriangleIndex * 3 + 2) = INDEX_NONE;
		TextureSpaceAdjacentTriangles(TriangleIndex * 3 + 0) = INDEX_NONE;
		TextureSpaceAdjacentTriangles(TriangleIndex * 3 + 1) = INDEX_NONE;
		TextureSpaceAdjacentTriangles(TriangleIndex * 3 + 2) = INDEX_NONE;
	}

	// Generate world space and texture space adjacency
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		for (INT OtherTriangleIndex = TriangleIndex + 1; OtherTriangleIndex < NumTriangles; OtherTriangleIndex++)
		{
			for (INT EdgeIndex = 0; EdgeIndex < 3; EdgeIndex++)
			{
				if (WorldSpaceAdjacentTriangles(TriangleIndex * 3 + EdgeIndex) == INDEX_NONE)
				{
					for (INT OtherEdgeIndex = 0; OtherEdgeIndex < 3; OtherEdgeIndex++)
					{
						if (WorldSpaceAdjacentTriangles(OtherTriangleIndex * 3 + OtherEdgeIndex) == INDEX_NONE)
						{
							const FStaticLightingVertex& V0 = MeshVertices(TriangleIndex * 3 + EdgeIndex);
							const FStaticLightingVertex& V1 = MeshVertices(TriangleIndex * 3 + (EdgeIndex + 1) % 3);
							const FStaticLightingVertex& OtherV0 = MeshVertices(OtherTriangleIndex * 3 + OtherEdgeIndex);
							const FStaticLightingVertex& OtherV1 = MeshVertices(OtherTriangleIndex * 3 + (OtherEdgeIndex + 1) % 3);
							// Triangles are adjacent if they share one edge in world space
							if ((V0.WorldPosition - OtherV1.WorldPosition).IsNearlyZero(KINDA_SMALL_NUMBER * 100.0f)
								&& (V1.WorldPosition - OtherV0.WorldPosition).IsNearlyZero(KINDA_SMALL_NUMBER * 100.0f))
							{
								WorldSpaceAdjacentTriangles(TriangleIndex * 3 + EdgeIndex) = OtherTriangleIndex;
								WorldSpaceAdjacentTriangles(OtherTriangleIndex * 3 + OtherEdgeIndex) = TriangleIndex;
								break;
							}
						}
					}
				}

				if (TextureSpaceAdjacentTriangles(TriangleIndex * 3 + EdgeIndex) == INDEX_NONE)
				{
					for (INT OtherEdgeIndex = 0; OtherEdgeIndex < 3; OtherEdgeIndex++)
					{
						if (TextureSpaceAdjacentTriangles(OtherTriangleIndex * 3 + OtherEdgeIndex) == INDEX_NONE)
						{
							const FStaticLightingVertex& V0 = MeshVertices(TriangleIndex * 3 + EdgeIndex);
							const FStaticLightingVertex& V1 = MeshVertices(TriangleIndex * 3 + (EdgeIndex + 1) % 3);
							const FStaticLightingVertex& OtherV0 = MeshVertices(OtherTriangleIndex * 3 + OtherEdgeIndex);
							const FStaticLightingVertex& OtherV1 = MeshVertices(OtherTriangleIndex * 3 + (OtherEdgeIndex + 1) % 3);
							// Triangles are adjacent if they share one edge in texture space
							if ((V0.TextureCoordinates[TextureCoordinateIndex] - OtherV1.TextureCoordinates[TextureCoordinateIndex]).IsNearlyZero(KINDA_SMALL_NUMBER * 100.0f)
								&& (V1.TextureCoordinates[TextureCoordinateIndex] - OtherV0.TextureCoordinates[TextureCoordinateIndex]).IsNearlyZero(KINDA_SMALL_NUMBER * 100.0f))
							{
								TextureSpaceAdjacentTriangles(TriangleIndex * 3 + EdgeIndex) = OtherTriangleIndex;
								TextureSpaceAdjacentTriangles(OtherTriangleIndex * 3 + OtherEdgeIndex) = TriangleIndex;
								break;
							}
						}
					}
				}
			}
		}
	}

	TArray<INT> TriangleGroups;
	TriangleGroups.Empty(NumTriangles);
	TriangleGroups.Add(NumTriangles);
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		TriangleGroups(TriangleIndex) = INDEX_NONE;
	}

	TArray<INT> PendingTriangles;
	INT NextGroupIndex = 0;
	// Arrange adjacent triangles in texture and world space together into groups
	// Assign a group index to each triangle
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		if (TriangleGroups(TriangleIndex) == INDEX_NONE)
		{
			// Push the current triangle
			PendingTriangles.AddItem(TriangleIndex);
			const INT CurrentGroupIndex = NextGroupIndex;
			NextGroupIndex++;
			while (PendingTriangles.Num() > 0)
			{
				// Pop the next pending triangle and process it
				const INT NeighborTriangleIndex = PendingTriangles.Pop();
				// Assign the group index
				TriangleGroups(NeighborTriangleIndex) = CurrentGroupIndex;
				// Flood fill all adjacent triangles with the same group index
				for (INT NeighborIndex = 0; NeighborIndex < 3; NeighborIndex++)
				{
					const INT WorldSpaceNeighbor = WorldSpaceAdjacentTriangles(NeighborTriangleIndex * 3 + NeighborIndex);
					const INT TextureSpaceNeighbor = TextureSpaceAdjacentTriangles(NeighborTriangleIndex * 3 + NeighborIndex);
					if (WorldSpaceNeighbor != INDEX_NONE 
						&& WorldSpaceNeighbor == TextureSpaceNeighbor
						&& ElementIndices(TriangleIndex) == ElementIndices(NeighborTriangleIndex)
						&& TriangleGroups(WorldSpaceNeighbor) == INDEX_NONE)
					{
						PendingTriangles.AddItem(WorldSpaceNeighbor);
					}
				}
			}
		}
	}

	TArray<TArray<INT> > GroupedTriangles;
	GroupedTriangles.Empty(NextGroupIndex);
	GroupedTriangles.AddZeroed(NextGroupIndex);
	for (INT TriangleIndex = 0; TriangleIndex < NumTriangles; TriangleIndex++)
	{
		const INT GroupIndex = TriangleGroups(TriangleIndex);
		GroupedTriangles(GroupIndex).AddItem(TriangleIndex);
	} 

	LayeredGroupTriangles.AddItem(GroupedTriangles(0));

	// At this point many meshes will have hundreds of groups, depending on how many UV charts they have
	// Merge these groups into the same layer to be processed together if they share the same material and are not overlapping in UV space
	for (INT GroupIndex = 1; GroupIndex < GroupedTriangles.Num(); GroupIndex++)
	{
		const INT GroupElementIndex = ElementIndices(GroupedTriangles(GroupIndex)(0));
		UBOOL bMergedGroup = FALSE;
		// Search through the merged groups for one that the current group can be merged into
		for (INT LayeredGroupIndex = 0; LayeredGroupIndex < LayeredGroupTriangles.Num() && !bMergedGroup; LayeredGroupIndex++)
		{
			const INT LayerGroupElementIndex = ElementIndices(LayeredGroupTriangles(LayeredGroupIndex)(0));
			if (GroupElementIndex == LayerGroupElementIndex)
			{
				UBOOL bOverlapping = FALSE;
				for (INT TriangleIndex = 0; TriangleIndex < GroupedTriangles(GroupIndex).Num() && !bOverlapping; TriangleIndex++)
				{
					for (INT OtherTriangleIndex = 0; OtherTriangleIndex < LayeredGroupTriangles(LayeredGroupIndex).Num(); OtherTriangleIndex++)
					{
						const FVector2D& V0 = MeshVertices(GroupedTriangles(GroupIndex)(TriangleIndex) * 3 + 0).TextureCoordinates[TextureCoordinateIndex];
						const FVector2D& V1 = MeshVertices(GroupedTriangles(GroupIndex)(TriangleIndex) * 3 + 1).TextureCoordinates[TextureCoordinateIndex];
						const FVector2D& V2 = MeshVertices(GroupedTriangles(GroupIndex)(TriangleIndex) * 3 + 2).TextureCoordinates[TextureCoordinateIndex];

						const FVector2D& OtherV0 = MeshVertices(LayeredGroupTriangles(LayeredGroupIndex)(OtherTriangleIndex) * 3 + 0).TextureCoordinates[TextureCoordinateIndex];
						const FVector2D& OtherV1 = MeshVertices(LayeredGroupTriangles(LayeredGroupIndex)(OtherTriangleIndex) * 3 + 1).TextureCoordinates[TextureCoordinateIndex];
						const FVector2D& OtherV2 = MeshVertices(LayeredGroupTriangles(LayeredGroupIndex)(OtherTriangleIndex) * 3 + 2).TextureCoordinates[TextureCoordinateIndex];

						if (AxisAlignedTriangleIntersectTriangle2d(V0, V1, V2, OtherV0, OtherV1, OtherV2))
						{
							bOverlapping = TRUE;
							break;
						}
					}
				}

				
				if (!bOverlapping)
				{
					// The current group was the same element index as the current merged group and they did not overlap in texture space, merge them
					bMergedGroup = TRUE;
					LayeredGroupTriangles(LayeredGroupIndex).Append(GroupedTriangles(GroupIndex));
				}
			}
		}
		if (!bMergedGroup)
		{
			// The current group did not get merged into any layers, add a new layer
			LayeredGroupTriangles.AddItem(GroupedTriangles(GroupIndex));
		}
	}
}

/** Adds an entry to Texels if the given texel passes the emissive criteria. */
void FStaticLightingMesh::AddLightTexel(
	const FTexelToCornersMap& TexelToCornersMap, 
	INT ElementIndex,
	TArray<INT>& LightIndices, 
	INT X, INT Y, 
	FLOAT EmissiveThreshold,
	TArray<FIntPoint>& Texels,
	INT TexSizeX, 
	INT TexSizeY) const
{
	if (X >= 0
		&& X < TexelToCornersMap.GetSizeX()
		&& Y >= 0
		&& Y < TexelToCornersMap.GetSizeY()
		// Only continue if this texel hasn't already been processed
		&& LightIndices(Y * TexelToCornersMap.GetSizeX() + X) == UnprocessedIndex)
	{
		const FTexelToCorners& CurrentTexelCorners = TexelToCornersMap(X, Y);
		UBOOL bAllCornersValid = TRUE;
		for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
		{
			bAllCornersValid = bAllCornersValid && CurrentTexelCorners.bValid[CornerIndex];
		}

		//@todo - handle partial texels
		if (bAllCornersValid)
		{
			const FLOAT XFraction = X / (FLOAT)TexSizeX;
			const FLOAT YFraction = Y / (FLOAT)TexSizeY;
			const FLinearColor CurrentEmissive = EvaluateEmissive(FVector2D(XFraction, YFraction), ElementIndex);
			if (CurrentEmissive.R > EmissiveThreshold || CurrentEmissive.G > EmissiveThreshold || CurrentEmissive.B > EmissiveThreshold)
			{
				Texels.AddItem(FIntPoint(X, Y));
				// Mark the texel as pending so it doesn't get added to Texels again
				LightIndices(Y * TexelToCornersMap.GetSizeX() + X) = PendingProcessingIndex;
				return;
			}
		}
		// Mark the texel as not emissive so we won't process it again
		LightIndices(Y * TexelToCornersMap.GetSizeX() + X) = NotEmissiveIndex;
	}
}

/** Adds an entry to Texels if the given texel passes the primitive simplifying criteria. */
void FStaticLightingMesh::AddPrimitiveTexel(
	const FTexelToCornersMap& TexelToCornersMap, 
	const FTexelToCorners& ComparisonTexel,
	INT ComparisonTexelLightIndex,
	const FVector4& PrimitiveOrigin,
	TArray<INT>& PrimitiveIndices, 
	const TArray<INT>& LightIndices, 
	INT X, INT Y, 
	TArray<FIntPoint>& Texels,
	const FScene& Scene,
	FLOAT DistanceThreshold) const
{
	if (X >= 0
		&& X < TexelToCornersMap.GetSizeX()
		&& Y >= 0
		&& Y < TexelToCornersMap.GetSizeY()
		// Only continue if this texel hasn't already been processed
		&& PrimitiveIndices(Y * TexelToCornersMap.GetSizeX() + X) == UnprocessedIndex)
	{
		const INT LightIndex = LightIndices(Y * TexelToCornersMap.GetSizeX() + X);
		if (LightIndex == NotEmissiveIndex)
		{
			// Mark the texel as not emissive so we won't process it again
			PrimitiveIndices(Y * TexelToCornersMap.GetSizeX() + X) = NotEmissiveIndex;
		}
		// Only assign this texel to the primitive if its light index matches the primitive's light index
		else if (LightIndex == ComparisonTexelLightIndex)
		{
			const FTexelToCorners& CurrentTexelCorners = TexelToCornersMap(X, Y);
			FVector4 PrimitiveCenter(0,0,0);
			for (INT CornerIndex = 0; CornerIndex < NumTexelCorners; CornerIndex++)
			{
				PrimitiveCenter += CurrentTexelCorners.Corners[CornerIndex].WorldPosition / (FLOAT)NumTexelCorners;
			}

			const FLOAT NormalsDot = CurrentTexelCorners.WorldTangentZ | ComparisonTexel.WorldTangentZ;
			const FLOAT DistanceToPrimitiveOriginSq = (PrimitiveCenter - PrimitiveOrigin).SizeSquared();
			// Only merge into the simplified primitive if this texel's normal is similar and it is within a distance threshold
			if (NormalsDot > Scene.MeshAreaLightSettings.MeshAreaLightSimplifyNormalCosAngleThreshold && DistanceToPrimitiveOriginSq < Square(DistanceThreshold))
			{
				UBOOL bAnyCornersMatch = FALSE;
				for (INT CornerIndex = 0; CornerIndex < NumTexelCorners && !bAnyCornersMatch; CornerIndex++)
				{
					const FVector4 CurrentPosition = CurrentTexelCorners.Corners[CornerIndex].WorldPosition;
					for (INT OtherCornerIndex = 0; OtherCornerIndex < NumTexelCorners; OtherCornerIndex++)
					{
						if ((CurrentPosition - ComparisonTexel.Corners[OtherCornerIndex].WorldPosition).SizeSquared() < Square(Scene.MeshAreaLightSettings.MeshAreaLightSimplifyCornerDistanceThreshold))
						{
							bAnyCornersMatch = TRUE;
							break;
						}
					}
				}

				// Only merge into the simplified primitive if any corner of this texel has the same position as the neighboring texel in the primitive
				if (bAnyCornersMatch)
				{
					Texels.AddItem(FIntPoint(X, Y));
					// Mark the texel as pending so it doesn't get added to Texels again
					PrimitiveIndices(Y * TexelToCornersMap.GetSizeX() + X) = PendingProcessingIndex;
					return;
				}
			}
		}
	}
}

} //namespace Lightmass
