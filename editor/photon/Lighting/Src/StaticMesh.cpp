/*=============================================================================
	StaticMesh.cpp: Static lighting StaticMesh mesh/mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Importer.h"

namespace Lightmass
{

/** 
 * Functions used for transforming a static mesh component based on a spline.  
 * This needs to be updated if the spline functionality changes in UE3!
 */

static FLOAT SmoothStep(FLOAT A, FLOAT B, FLOAT X)
{
	if (X < A)
	{
		return 0.0f;
	}
	else if (X >= B)
	{
		return 1.0f;
	}
	const FLOAT InterpFraction = (X - A) / (B - A);
	return InterpFraction * InterpFraction * (3.0f - 2.0f * InterpFraction);
}

static FVector4 SplineEvalPos(const FVector4& StartPos, const FVector4& StartTangent, const FVector4& EndPos, const FVector4& EndTangent, FLOAT A)
{
	const FLOAT A2 = A  * A;
	const FLOAT A3 = A2 * A;

	return (((2*A3)-(3*A2)+1) * StartPos) + ((A3-(2*A2)+A) * StartTangent) + ((A3-A2) * EndTangent) + (((-2*A3)+(3*A2)) * EndPos);
}

static FVector4 SplineEvalDir(const FVector4& StartPos, const FVector4& StartTangent, const FVector4& EndPos, const FVector4& EndTangent, FLOAT A)
{
	const FVector4 C = (6*StartPos) + (3*StartTangent) + (3*EndTangent) - (6*EndPos);
	const FVector4 D = (-6*StartPos) - (4*StartTangent) - (2*EndTangent) + (6*EndPos);
	const FVector4 E = StartTangent;

	const FLOAT A2 = A  * A;

	return ((C * A2) + (D * A) + E).SafeNormal();
}

/** Calculate full transform that defines frame along spline, given the Z of a vertex. */
/** Note:  This is mirrored from USplineMeshComponent::CalcSliceTransform() and LocalVertexShader.usf.  If you update one of these, please update them all! */
static FMatrix CalcSliceTransform(FLOAT ZPos, const FSplineMeshParams& SplineParams)
{
	// Find how far 'along' mesh we are
	const FLOAT Alpha = (ZPos - SplineParams.MeshMinZ)/SplineParams.MeshRangeZ;

	// Apply hermite interp to Alpha if desired
	const FLOAT HermiteAlpha = SplineParams.bSmoothInterpRollScale ? SmoothStep(0.0, 1.0, Alpha) : Alpha;

	// Then find the point and direction of the spline at this point along
	FVector4 SplinePos = SplineEvalPos( SplineParams.StartPos, SplineParams.StartTangent, SplineParams.EndPos, SplineParams.EndTangent, Alpha );	
	const FVector4 SplineDir = SplineEvalDir( SplineParams.StartPos, SplineParams.StartTangent, SplineParams.EndPos, SplineParams.EndTangent, Alpha );

	// Find base frenet frame
	const FVector4 BaseYVec = (SplineDir ^ SplineParams.SplineXDir).SafeNormal();
	const FVector4 BaseXVec = (BaseYVec ^ SplineDir).SafeNormal();	

	// Offset the spline by the desired amount
	const FVector2D SliceOffset = Lerp<FVector2D>(SplineParams.StartOffset, SplineParams.EndOffset, HermiteAlpha);
	SplinePos += SliceOffset.X * BaseXVec;
	SplinePos += SliceOffset.Y * BaseYVec;

	// Apply roll to frame around spline	
	const FLOAT UseRoll = Lerp(SplineParams.StartRoll, SplineParams.EndRoll, HermiteAlpha);
	const FLOAT CosAng = appCos(UseRoll);
	const FLOAT SinAng = appSin(UseRoll);
	const FVector4 XVec = (CosAng * BaseXVec) - (SinAng * BaseYVec);
	const FVector4 YVec = (CosAng * BaseYVec) + (SinAng * BaseXVec);

	// Find scale at this point along spline
	const FVector2D UseScale = Lerp(SplineParams.StartScale, SplineParams.EndScale, HermiteAlpha);

	// Build overall transform
	const FMatrix SliceTransform(UseScale.X * FVector4(XVec,0), UseScale.Y * FVector4(YVec,0), FVector4(SplineDir,0), FVector4(SplinePos,1));

	return SliceTransform;
}

/** Calculate rotation matrix that defines frame along spline, given the Z of a vertex. */
static FMatrix CalcSliceRot(FLOAT ZPos, const FSplineMeshParams& SplineParams)
{
	// Find how far 'along' mesh we are
	const FLOAT Alpha = (ZPos - SplineParams.MeshMinZ)/SplineParams.MeshRangeZ;

	// Apply hermite interp to Alpha if desired
	const FLOAT HermiteAlpha = SplineParams.bSmoothInterpRollScale ? SmoothStep(0.0, 1.0, Alpha) : Alpha;

	// Then find the point and direction of the spline at this point along
	const FVector4 SplineDir = SplineEvalDir( SplineParams.StartPos, SplineParams.StartTangent, SplineParams.EndPos, SplineParams.EndTangent, Alpha );

	// Find base frenet frame
	const FVector4 BaseYVec = (SplineDir ^ SplineParams.SplineXDir).SafeNormal();
	const FVector4 BaseXVec = (BaseYVec ^ SplineDir).SafeNormal();	

	// Apply roll to frame around spline
	const FLOAT UseRoll = Lerp(SplineParams.StartRoll, SplineParams.EndRoll, HermiteAlpha);
	const FLOAT CosAng = appCos(UseRoll);
	const FLOAT SinAng = appSin(UseRoll);
	const FVector4 XVec = (CosAng * BaseXVec) - (SinAng * BaseYVec);
	const FVector4 YVec = (CosAng * BaseYVec) + (SinAng * BaseXVec);

	// Build rotation transform
	const FMatrix SliceTransform(XVec, YVec, SplineDir, FVector4(0,0,0,1));

	return SliceTransform;
}

/**
 * Creates a static lighting vertex to represent the given static mesh vertex.
 * @param VertexBuffer - The static mesh's vertex buffer.
 * @param VertexIndex - The index of the static mesh vertex to access.
 * @param OutVertex - Upon return, contains a static lighting vertex representing the specified static mesh vertex.
 */
static void GetStaticLightingVertex(
	const FStaticMeshVertex& InVertex,
	const FMatrix& LocalToWorld,
	const FMatrix& LocalToWorldInverseTranspose,
	UBOOL bIsSplineMesh,
	const FSplineMeshParams& SplineParams,
	FStaticLightingVertex& OutVertex
	)
{
	if (bIsSplineMesh)
	{
		// Make transform for this point along spline
		const FMatrix SliceTransform = CalcSliceTransform(InVertex.Position.Z, SplineParams);

		// Remove Z (transform will move us along spline)
		// Transform into mesh space
		const FVector4 LocalPos = SliceTransform.TransformFVector(FVector4(InVertex.Position.X, InVertex.Position.Y, 0.0f));

		// Transform from mesh to world space
		OutVertex.WorldPosition = LocalToWorld.TransformFVector(LocalPos);

		const FMatrix SliceRot = CalcSliceRot(InVertex.Position.Z, SplineParams);

		const FVector4 LocalSpaceTangentX = SliceRot.TransformNormal(InVertex.TangentX);
		const FVector4 LocalSpaceTangentY = SliceRot.TransformNormal(InVertex.TangentY);
		const FVector4 LocalSpaceTangentZ = SliceRot.TransformNormal(InVertex.TangentZ);

		OutVertex.WorldTangentX = LocalToWorld.TransformNormal(LocalSpaceTangentX).SafeNormal();
		OutVertex.WorldTangentY = LocalToWorld.TransformNormal(LocalSpaceTangentY).SafeNormal();
		OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(LocalSpaceTangentZ).SafeNormal();
	}
	else
	{
		OutVertex.WorldPosition = LocalToWorld.TransformFVector(InVertex.Position);
		OutVertex.WorldTangentX = LocalToWorld.TransformNormal(InVertex.TangentX).SafeNormal();
		OutVertex.WorldTangentY = LocalToWorld.TransformNormal(InVertex.TangentY).SafeNormal();
		OutVertex.WorldTangentZ = LocalToWorldInverseTranspose.TransformNormal(InVertex.TangentZ).SafeNormal();
	}

	// WorldTangentZ can end up a 0 vector if it was small to begin with and LocalToWorld contains large scale factors.
	if (!OutVertex.WorldTangentZ.IsUnit())
	{
		OutVertex.WorldTangentZ = (OutVertex.WorldTangentX ^ OutVertex.WorldTangentY).SafeNormal();
	}

	for(UINT LightmapTextureCoordinateIndex = 0; LightmapTextureCoordinateIndex < ARRAY_COUNT(InVertex.UVs); LightmapTextureCoordinateIndex++)
	{
		OutVertex.TextureCoordinates[LightmapTextureCoordinateIndex] = InVertex.UVs[LightmapTextureCoordinateIndex];
	}
}

// FStaticLightingMesh interface.

void FStaticMeshStaticLightingMesh::GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
{
 	const FStaticMeshLOD& LODRenderData = StaticMesh->GetLOD(GetMeshLODIndex());
 
 	// Lookup the triangle's vertex indices.
 	const WORD I0 = LODRenderData.GetIndex(TriangleIndex * 3 + 0);
 	const WORD I1 = LODRenderData.GetIndex(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
 	const WORD I2 = LODRenderData.GetIndex(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));
 
 	// Translate the triangle's static mesh vertices to static lighting vertices.
 	GetStaticLightingVertex(LODRenderData.GetVertex(I0), LocalToWorld, LocalToWorldInverseTranspose, bIsSplineMesh, SplineParameters, OutV0);
 	GetStaticLightingVertex(LODRenderData.GetVertex(I1), LocalToWorld, LocalToWorldInverseTranspose, bIsSplineMesh, SplineParameters, OutV1);
 	GetStaticLightingVertex(LODRenderData.GetVertex(I2), LocalToWorld, LocalToWorldInverseTranspose, bIsSplineMesh, SplineParameters, OutV2);

	const FStaticMeshLOD& MeshLOD = StaticMesh->GetLOD(GetMeshLODIndex());
	ElementIndex = INDEX_NONE;
	for (UINT MeshElementIndex = 0; MeshElementIndex < MeshLOD.NumElements; MeshElementIndex++)
	{
		const FStaticMeshElement& CurrentElement = MeshLOD.GetElement(MeshElementIndex);
		if ((UINT)TriangleIndex >= CurrentElement.FirstIndex / 3 && (UINT)TriangleIndex < CurrentElement.FirstIndex / 3 + CurrentElement.NumTriangles)
		{
			ElementIndex = MeshElementIndex;
			break;
		}
	}
	check(ElementIndex >= 0);
}

void FStaticMeshStaticLightingMesh::GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
{
 	const FStaticMeshLOD& LODRenderData = StaticMesh->GetLOD(GetMeshLODIndex());
 
 	// Lookup the triangle's vertex indices.
 	OutI0 = LODRenderData.GetIndex(TriangleIndex * 3 + 0);
 	OutI1 = LODRenderData.GetIndex(TriangleIndex * 3 + (bReverseWinding ? 2 : 1));
 	OutI2 = LODRenderData.GetIndex(TriangleIndex * 3 + (bReverseWinding ? 1 : 2));
}

UBOOL FStaticMeshStaticLightingMesh::IsElementCastingShadow(INT ElementIndex) const
{
	const FStaticMeshLOD& LODRenderData = StaticMesh->GetLOD(GetMeshLODIndex());
	const FStaticMeshElement& Element = LODRenderData.GetElement( ElementIndex );
	return Element.bEnableShadowCasting;
}

void FStaticMeshStaticLightingMesh::Read( FILE* FileHandle, FScene& Scene)
{
	FStaticLightingMesh::Read( FileHandle, Scene);

	// import the shared data
	FStaticMeshStaticLightingMeshData TempSMSLMD;
	fread( &TempSMSLMD, sizeof(FStaticMeshStaticLightingMeshData), 1, FileHandle);

	EncodedLODIndex = TempSMSLMD.EncodedLODIndex;
	LocalToWorld = TempSMSLMD.LocalToWorld;
	bReverseWinding = TempSMSLMD.bReverseWinding;
	bShouldSelfShadow = TempSMSLMD.bShouldSelfShadow;
	StaticMeshGuid = TempSMSLMD.StaticMeshGuid;
	bIsSplineMesh = TempSMSLMD.bIsSplineMesh;
	SplineParameters = TempSMSLMD.SplineParameters;

	// calculate the inverse transpose
	LocalToWorldInverseTranspose = LocalToWorld.Inverse().Transpose();

	// we have the guid for the mesh, now hook it up to the actual static mesh
	StaticMesh = Scene.GetStaticMesh( StaticMeshGuid);
	checkf(StaticMesh, TEXT("Failed to import static mesh with GUID %s"), *StaticMeshGuid.String());
	check(GetMeshLODIndex() >= 0 && GetMeshLODIndex() < (INT)StaticMesh->NumLODs);
	checkf(StaticMesh->GetLOD(GetMeshLODIndex()).NumElements == MaterialElements.Num(), TEXT("Static mesh element count did not match mesh instance element count!"));
}

void FStaticMeshStaticLightingMesh::Import( FLightmassImporter& Importer )
{
	// import super class
	FStaticLightingMesh::Import( Importer );

	// import the shared data
//	Importer.ImportData( (FStaticMeshStaticLightingMeshData*) this );
	FStaticMeshStaticLightingMeshData TempSMSLMD;
	Importer.ImportData(&TempSMSLMD);
	EncodedLODIndex = TempSMSLMD.EncodedLODIndex;
	LocalToWorld = TempSMSLMD.LocalToWorld;
	bReverseWinding = TempSMSLMD.bReverseWinding;
	bShouldSelfShadow = TempSMSLMD.bShouldSelfShadow;
	StaticMeshGuid = TempSMSLMD.StaticMeshGuid;
	bIsSplineMesh = TempSMSLMD.bIsSplineMesh;
	SplineParameters = TempSMSLMD.SplineParameters;

	// calculate the inverse transpose
	LocalToWorldInverseTranspose = LocalToWorld.Inverse().Transpose();

	// we have the guid for the mesh, now hook it up to the actual static mesh
	StaticMesh = Importer.ConditionalImportObject<FStaticMesh>(StaticMeshGuid, LM_STATICMESH_VERSION, LM_STATICMESH_EXTENSION, LM_STATICMESH_CHANNEL_FLAGS, Importer.GetStaticMeshes());
	checkf(StaticMesh, TEXT("Failed to import static mesh with GUID %s"), *StaticMeshGuid.String());
	check(GetMeshLODIndex() >= 0 && GetMeshLODIndex() < (INT)StaticMesh->NumLODs);
	checkf(StaticMesh->GetLOD(GetMeshLODIndex()).NumElements == MaterialElements.Num(), TEXT("Static mesh element count did not match mesh instance element count!"));
}

void FStaticMeshStaticLightingTextureMapping::Read( FILE* FileHandle, FScene& Scene)
{
	FStaticLightingTextureMapping::Read( FileHandle, Scene);

	check(Mesh);
}

void FStaticMeshStaticLightingTextureMapping::Import( FLightmassImporter& Importer )
{
	// import the super class
	FStaticLightingTextureMapping::Import(Importer);
	check(Mesh);
}

void FStaticMeshStaticLightingVertexMapping::Import( FLightmassImporter& Importer )
{
	// import the super class
	FStaticLightingVertexMapping::Import(Importer);
	check(Mesh);
}



} //namespace Lightmass
