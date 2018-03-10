/*=============================================================================
	LightingMesh.h: Static lighting mesh definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "ImportExport.h"

namespace Lightmass
{

/** The vertex data used to build static lighting. */
struct FStaticLightingVertex: public FStaticLightingVertexData
{
	/** Transforms a world space vector into the tangent space of this vertex. */
	inline FVector4 TransformWorldVectorToTangent(const FVector4& WorldVector) const
	{
		const FVector4 TangentVector(
			WorldTangentX | WorldVector,
			WorldTangentY | WorldVector,
			WorldTangentZ | WorldVector
			);
		return TangentVector;
	}

	/** Transforms a vector in the tangent space of this vertex into world space. */
	inline FVector4 TransformTangentVectorToWorld(const FVector4& TangentVector) const
	{
		checkSlow(TangentVector.IsUnit());
		// Assuming the transpose of the tangent basis is also the inverse
		const FVector4 WorldTangentRow0(WorldTangentX.X, WorldTangentY.X, WorldTangentZ.X);
		const FVector4 WorldTangentRow1(WorldTangentX.Y, WorldTangentY.Y, WorldTangentZ.Y);
		const FVector4 WorldTangentRow2(WorldTangentX.Z, WorldTangentY.Z, WorldTangentZ.Z);
		const FVector4 WorldVector(
			WorldTangentRow0 | TangentVector,
			WorldTangentRow1 | TangentVector,
			WorldTangentRow2 | TangentVector
			);
		checkSlow(WorldVector.IsUnit());
		return WorldVector;
	}

	/** Generates WorldTangentX and WorldTangentY from WorldTangentZ such that the tangent basis is orthonormal. */
	inline void GenerateVertexTangents()
	{
		checkSlow(WorldTangentZ.IsUnit());
		// Use the vector perpendicular to the normal and the negative Y axis as the TangentX.  
		// A WorldTangentZ of (0,0,1) will generate WorldTangentX of (1,0,0) and WorldTangentY of (0,1,0) which can be useful for debugging tangent space issues.
		const FVector4 TangentXCandidate = WorldTangentZ ^ FVector4(0,-1,0);
		if (TangentXCandidate.SizeSquared() < KINDA_SMALL_NUMBER)
		{
			// The normal was nearly equal to the Y axis, use the X axis instead
			WorldTangentX = (WorldTangentZ ^ FVector4(1,0,0)).UnsafeNormal();
		}
		else
		{
			WorldTangentX = TangentXCandidate.UnsafeNormal();
		}
		WorldTangentY = WorldTangentZ ^ WorldTangentX;
		checkSlow(WorldTangentY.IsUnit());
	}

	// Operators used for linear combinations of static lighting vertices.
	friend FStaticLightingVertex operator+(const FStaticLightingVertex& A,const FStaticLightingVertex& B)
	{
		FStaticLightingVertex Result;
		Result.WorldPosition =	A.WorldPosition + B.WorldPosition;
		Result.WorldTangentX =	A.WorldTangentX + B.WorldTangentX;
		Result.WorldTangentY =	A.WorldTangentY + B.WorldTangentY;
		Result.WorldTangentZ =	A.WorldTangentZ + B.WorldTangentZ;
		for(INT CoordinateIndex = 0;CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			Result.TextureCoordinates[CoordinateIndex] = A.TextureCoordinates[CoordinateIndex] + B.TextureCoordinates[CoordinateIndex];
		}
		return Result;
	}

	friend FStaticLightingVertex operator-(const FStaticLightingVertex& A,const FStaticLightingVertex& B)
	{
		FStaticLightingVertex Result;
		Result.WorldPosition =	A.WorldPosition - B.WorldPosition;
		Result.WorldTangentX =	A.WorldTangentX - B.WorldTangentX;
		Result.WorldTangentY =	A.WorldTangentY - B.WorldTangentY;
		Result.WorldTangentZ =	A.WorldTangentZ - B.WorldTangentZ;
		for(INT CoordinateIndex = 0;CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			Result.TextureCoordinates[CoordinateIndex] = A.TextureCoordinates[CoordinateIndex] - B.TextureCoordinates[CoordinateIndex];
		}
		return Result;
	}

	friend FStaticLightingVertex operator*(const FStaticLightingVertex& A,FLOAT B)
	{
		FStaticLightingVertex Result;
		Result.WorldPosition =	A.WorldPosition * B;
		Result.WorldTangentX =	A.WorldTangentX * B;
		Result.WorldTangentY =	A.WorldTangentY * B;
		Result.WorldTangentZ =	A.WorldTangentZ * B;
		for(INT CoordinateIndex = 0;CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			Result.TextureCoordinates[CoordinateIndex] = A.TextureCoordinates[CoordinateIndex] * B;
		}
		return Result;
	}

	friend FStaticLightingVertex operator/(const FStaticLightingVertex& A,FLOAT B)
	{
		const FLOAT InvB = 1.0f / B;

		FStaticLightingVertex Result;
		Result.WorldPosition =	A.WorldPosition * InvB;
		Result.WorldTangentX =	A.WorldTangentX * InvB;
		Result.WorldTangentY =	A.WorldTangentY * InvB;
		Result.WorldTangentZ =	A.WorldTangentZ * InvB;
		for(INT CoordinateIndex = 0;CoordinateIndex < MAX_TEXCOORDS;CoordinateIndex++)
		{
			Result.TextureCoordinates[CoordinateIndex] = A.TextureCoordinates[CoordinateIndex] * InvB;
		}
		return Result;
	}
};

/** The result of an intersection between a light ray and the scene. */
class FLightRayIntersection
{
public:

	/** TRUE if the light ray intersected opaque scene geometry. */
	BITFIELD bIntersects : 1;

	/** The differential geometry which the light ray intersected with, only valid if the ray intersected. */
	FStaticLightingVertex IntersectionVertex;

	/** Transmission of the ray, valid whether the ray intersected or not as long as Transmission was requested from FStaticLightingAggregateMesh::IntersectLightRay. */
	FLinearColor Transmission;

	/** The mesh that was intersected by the ray, only valid if the ray intersected. */
	const class FStaticLightingMesh* Mesh;

	/** The mapping that was intersected by the ray, only valid if the ray intersected. */
	const class FStaticLightingMapping* Mapping;

	/** Index of one of the vertices in the triangle intersected. */
	INT VertexIndex;

	/** Primitive type specific element index associated with the triangle that was hit, only valid if the ray intersected. */
	INT ElementIndex;

	/** Dummy constructor, not initializing any members. */
	FLightRayIntersection()
	{}

	/** Initialization constructor. */
	FLightRayIntersection(
		UBOOL bInIntersects, 
		const FStaticLightingVertex& InIntersectionVertex, 
		const FStaticLightingMesh* InMesh, 
		const FStaticLightingMapping* InMapping,
		INT InVertexIndex,
		INT InElementIndex)
		:
		bIntersects(bInIntersects),
		IntersectionVertex(InIntersectionVertex),
		Mesh(InMesh),
		Mapping(InMapping),
		VertexIndex(InVertexIndex),
		ElementIndex(InElementIndex)
	{
		checkSlow(!bInIntersects || (InMesh && /*InMapping &&*/ ElementIndex >= 0));
	}

	/** No intersection constructor. */
	static FLightRayIntersection None() { return FLightRayIntersection(FALSE,FStaticLightingVertex(),NULL,NULL,INDEX_NONE,INDEX_NONE); }
};

/** Stores information about an element of the mesh which can have its own material. */
class FMaterialElement : public FMaterialElementData
{
public:
	/** Whether Material has transmission, cached here to avoid dereferencing Material. */
	UBOOL bTranslucent;
	/** Whether Material is Masked, cached here to avoid dereferencing Material. */
	UBOOL bIsMasked;
	/** 
	 * Whether Material is TwoSided, cached here to avoid dereferencing Material. 
	 * This is different from FMaterialElementData::bUseTwoSidedLighting, because a two sided material may still want to use one sided lighting for the most part.
	 * It just indicates whether backfaces will be visible, and therefore artifacts on backfaces should be avoided.
	 */
	UBOOL bIsTwoSided;
	/** Whether Material wants to cast shadows as masked, cached here to avoid dereferencing Material. */
	UBOOL bCastShadowAsMasked;

	/** The material associated with this element.  After import, Material is always valid (non-null and points to an FMaterial). */
	FMaterial* Material;

	FMaterialElement() :
		bTranslucent(FALSE),
		bIsMasked(FALSE),
		bIsTwoSided(FALSE),
		bCastShadowAsMasked(FALSE),
		Material(NULL)
	{}
};

/** A mesh which is used for computing static lighting. */
class FStaticLightingMesh : public virtual FRefCountedObject, public FStaticLightingMeshInstanceData
{
public:
	/** The lights which affect the mesh's primitive. */
	TArray<FLight*> RelevantLights;

	/** 
	 * Visibility Id's corresponding to this static lighting mesh.  
	 * Has to be an array because BSP exports FStaticLightingMesh's per combined group of surfaces that should be lit together, 
	 * Instead of per-component geometry that should be visibility culled together.
	 */
	TArray<INT> VisibilityIds;

protected:

	/** Whether to color texels whose lightmap UV's are invalid. */
	UBOOL bColorInvalidTexels;

	/** Indicates whether DebugDiffuse, DebugSpecular and DebugSpecularPower should override the materials associated with this mesh. */
	UBOOL bUseDebugMaterial;
	FLinearColor DebugDiffuse;
	FLinearColor DebugSpecular;
	FLOAT DebugSpecularPower;

	/** 
	 * Materials used by the mesh, guaranteed to contain at least one. 
	 * These are indexed by the primitive type specific ElementIndex.
	 */
	TArray<FMaterialElement, TInlineAllocator<5>> MaterialElements;

	/** 
	 * Map from FStaticLightingMesh to the index given to uniquely identify all instances of the same primitive component.
	 * This is used to give all LOD's of the same primitive component the same mesh index.
	 */
	static TMap<FStaticLightingMesh*, INT> MeshToIndexMap;

public:

	/** Virtual destructor. */
	virtual ~FStaticLightingMesh() {}

	/** Returns whether the given element index is translucent. */
	inline UBOOL IsTranslucent(INT ElementIndex) const { return MaterialElements(ElementIndex).bTranslucent; }
	/** Returns whether the given element index is masked. */
	inline UBOOL IsMasked(INT ElementIndex) const { return MaterialElements(ElementIndex).bIsMasked; }
	/** Whether samples using the given element accept lighting from both sides of the triangle. */
	inline UBOOL UsesTwoSidedLighting(INT ElementIndex) const { return MaterialElements(ElementIndex).bUseTwoSidedLighting; }
	/** Whether samples using the given element are going to have backfaces visible, and therefore artifacts on backfaces should be avoided. */
	inline UBOOL IsTwoSided(INT ElementIndex) const { return MaterialElements(ElementIndex).bIsTwoSided || MaterialElements(ElementIndex).bUseTwoSidedLighting; }
	inline UBOOL IsCastingShadowsAsMasked(INT ElementIndex) const { return MaterialElements(ElementIndex).bCastShadowAsMasked; }
	inline UBOOL IsCastingShadowAsTwoSided() const { return bCastShadowAsTwoSided; }
	inline UBOOL IsEmissive(INT ElementIndex) const { return MaterialElements(ElementIndex).bUseEmissiveForStaticLighting; }
	inline UBOOL IsIndirectlyShadowedOnly(INT ElementIndex) const { return MaterialElements(ElementIndex).bShadowIndirectOnly; }
	inline FLOAT GetFullyOccludedSamplesFraction(INT ElementIndex) const { return MaterialElements(ElementIndex).FullyOccludedSamplesFraction; }
	inline INT GetNumElements() const { return MaterialElements.Num(); }
	inline UBOOL ShouldColorInvalidTexels() const { return bColorInvalidTexels; }

	/**
	 *	Returns the Guid for the object associated with this lighting mesh.
	 *	Ie, for a StaticMeshStaticLightingMesh, it would return the Guid of the source static mesh.
	 *	The GetObjectType function should also be used to determine the TypeId of the source object.
	 */
	virtual FGuid GetObjectGuid() const { return FGuid(0,0,0,0); }

	/**
	 *	Returns the SourceObject type id.
	 */
	virtual ESourceObjectType GetObjectType() const { return SOURCEOBJECTTYPE_Unknown; }

	/**
	 * Accesses a triangle for visibility testing.
	 * @param TriangleIndex - The triangle to access, valid range is [0, NumTriangles).
	 * @param OutV0 - Upon return, should contain the first vertex of the triangle.
	 * @param OutV1 - Upon return, should contain the second vertex of the triangle.
	 * @param OutV2 - Upon return, should contain the third vertex of the triangle.
	 * @param ElementIndex - Indicates the element index of the triangle.
     */
	virtual void GetTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const = 0;

	/**
	 * Accesses a triangle for shading.
	 * @param TriangleIndex - The triangle to access, valid range is [0, NumShadingTriangles).
	 * @param OutV0 - Upon return, should contain the first vertex of the triangle.
	 * @param OutV1 - Upon return, should contain the second vertex of the triangle.
	 * @param OutV2 - Upon return, should contain the third vertex of the triangle.
	 * @param ElementIndex - Indicates the element index of the triangle.
     */
	virtual void GetShadingTriangle(INT TriangleIndex,FStaticLightingVertex& OutV0,FStaticLightingVertex& OutV1,FStaticLightingVertex& OutV2,INT& ElementIndex) const
	{
		checkSlow(NumTriangles == NumShadingTriangles);
		// By default the geometry used for shading is the same as the geometry used for visibility testing.
		GetTriangle(TriangleIndex, OutV0, OutV1, OutV2, ElementIndex);
	}

	/**
	 * Accesses a triangle's vertex indices for visibility testing.
	 * @param TriangleIndex - The triangle to access, valid range is [0, NumTriangles).
	 * @param OutI0 - Upon return, should contain the first vertex index of the triangle.
	 * @param OutI1 - Upon return, should contain the second vertex index of the triangle.
	 * @param OutI2 - Upon return, should contain the third vertex index of the triangle.
	 */
	virtual void GetTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const = 0;

	/**
	 * Accesses a triangle's vertex indices for shading.
	 * @param TriangleIndex - The triangle to access, valid range is [0, NumShadingTriangles).
	 * @param OutI0 - Upon return, should contain the first vertex index of the triangle.
	 * @param OutI1 - Upon return, should contain the second vertex index of the triangle.
	 * @param OutI2 - Upon return, should contain the third vertex index of the triangle.
	 */
	virtual void GetShadingTriangleIndices(INT TriangleIndex,INT& OutI0,INT& OutI1,INT& OutI2) const
	{ 
		checkSlow(NumTriangles == NumShadingTriangles);
		// By default the geometry used for shading is the same as the geometry used for visibility testing.
		GetTriangleIndices(TriangleIndex, OutI0, OutI1, OutI2);
	}

	virtual UBOOL IsElementCastingShadow(INT ElementIndex) const 
	{ return TRUE; }

	/** Returns the LOD of this instance. */
	virtual INT GetLODIndex() const { return 0; }

	/** For debugging */
	virtual void SetDebugMaterial(UBOOL bUseDebugMaterial, FLinearColor Diffuse, FLinearColor Specular, FLOAT SpecularPower);

	/** Evaluates the mesh's Bidirectional Reflectance Distribution Function. */
	FLinearColor EvaluateBRDF(
		const FStaticLightingVertex& Vertex, 
		INT ElementIndex,
		const FVector4& IncomingDirection, 
		const FVector4& OutgoingDirection) const;

	/** Generates an outgoing direction sample and evaluates the BRDF for that direction. */
	FLinearColor SampleBRDF(
		const FStaticLightingVertex& Vertex, 
		INT ElementIndex,
		const FVector4& IncomingDirection, 
		FVector4& OutgoingDirection,
		FLOAT& DirectionPDF,
		UBOOL& bSpecularSample,
		FRandomStream& RandomStream
		) const;

	/** Evaluates the mesh's emissive at the given UVs */
	inline FLinearColor EvaluateEmissive(const FVector2D& UVs, INT ElementIndex) const
	{
		checkSlow(IsEmissive(ElementIndex)); 
		FLinearColor Emissive(FLinearColor::Black);
		FLOAT MaterialEmissiveBoost;
		const FMaterialElement& MaterialElement = MaterialElements(ElementIndex);
		MaterialElement.Material->SampleEmissive(UVs, Emissive, MaterialEmissiveBoost);
		FLinearColor EmissiveXYZ = Emissive.LinearRGBToXYZ();
		FLinearColor EmissivexyzY = EmissiveXYZ.XYZToxyzY();
		// Apply EmissiveBoost to the emissive brightness, which is Y in xyzY
		// Modifying brightness in xyzY to be consistent with DiffuseBoost and SpecularBoost
		EmissivexyzY.A = EmissivexyzY.A * MaterialEmissiveBoost * MaterialElement.EmissiveBoost;
		EmissiveXYZ = EmissivexyzY.xyzYToXYZ();
		Emissive = EmissiveXYZ.XYZToLinearRGB();
		return Emissive;
	}

	/** Evaluates the mesh's diffuse at the given UVs */
	inline FLinearColor EvaluateDiffuse(const FVector2D& UVs, INT ElementIndex) const
	{
		checkSlow(!IsTranslucent(ElementIndex));
		FLinearColor Diffuse(DebugDiffuse);
		if (!bUseDebugMaterial)
		{
			FLOAT MaterialDiffuseBoost;
			const FMaterialElement& MaterialElement = MaterialElements(ElementIndex);
			MaterialElement.Material->SampleDiffuse(UVs, Diffuse, MaterialDiffuseBoost);
			Diffuse.R = Max(Diffuse.R, 0.0f);
			Diffuse.G = Max(Diffuse.G, 0.0f);
			Diffuse.B = Max(Diffuse.B, 0.0f);
			FLinearColor DiffuseXYZ = Diffuse.LinearRGBToXYZ();
			FLinearColor DiffusexyzY = DiffuseXYZ.XYZToxyzY();
			// Apply DiffuseBoost to the diffuse brightness, which is Y in xyzY
			// Using xyzY allows us to modify the brightness of the color without changing the hue
			// Clamp diffuse to be physically valid for the modified Phong lighting model
			DiffusexyzY.A = Min(DiffusexyzY.A * MaterialDiffuseBoost * MaterialElement.DiffuseBoost, 1.0f);
			DiffuseXYZ = DiffusexyzY.xyzYToXYZ();
			Diffuse = DiffuseXYZ.XYZToLinearRGB();
		}
		return Diffuse;
	}

	/** Evaluates the mesh's specular and specular power at the given UVs */
	inline FLinearColor EvaluateSpecularAndPower(const FVector2D& UVs, INT ElementIndex) const
	{
		checkSlow(!IsTranslucent(ElementIndex));
		FLinearColor SpecularAndPower(DebugSpecular);
		SpecularAndPower.A = DebugSpecularPower;
		if (!bUseDebugMaterial)
		{
			FLOAT MaterialSpecularBoost;
			const FMaterialElement& MaterialElement = MaterialElements(ElementIndex);
			if (MaterialElement.Material->SpecularSize)
			{
				MaterialElement.Material->SampleSpecularAndPower(UVs, SpecularAndPower, MaterialSpecularBoost);
				FLinearColor SpecularRGB(Max(SpecularAndPower.R, 0.0f), Max(SpecularAndPower.G, 0.0f), Max(SpecularAndPower.B, 0.0f), 0.0f);
				FLinearColor SpecularXYZ = SpecularRGB.LinearRGBToXYZ();
				FLinearColor SpecularxyzY = SpecularXYZ.XYZToxyzY();
				// Apply SpecularBoost to the specular brightness, which is Y in xyzY
				// Using xyzY allows us to clamp the brightness of the color without changing the hue
				// Clamp specular to be physically valid for the modified Phong lighting model
				SpecularxyzY.A = Min(SpecularxyzY.A * MaterialSpecularBoost * MaterialElement.SpecularBoost, 1.0f);
				SpecularXYZ = SpecularxyzY.xyzYToXYZ();
				SpecularRGB = SpecularXYZ.XYZToLinearRGB();
				// Clamp specular power to be valid
				SpecularAndPower.A = Max(SpecularAndPower.A, 1.0f);
				SpecularAndPower = FLinearColor(SpecularRGB.R, SpecularRGB.G, SpecularRGB.B, SpecularAndPower.A);
			}
		}
		return SpecularAndPower;
	}

	/** Evaluates the mesh's transmission at the given UVs */
	inline FLinearColor EvaluateTransmission(const FVector2D& UVs, INT ElementIndex) const
	{
		checkSlow(IsTranslucent(ElementIndex));
		const FLinearColor Transmission = MaterialElements(ElementIndex).Material->SampleTransmission(UVs);
		return Transmission;
	}

	/** Evaluates the mesh's transmission at the given UVs */
	inline UBOOL EvaluateMaskedCollision(const FVector2D& UVs, INT ElementIndex) const
	{
		checkSlow(IsMasked(ElementIndex) || IsCastingShadowsAsMasked(ElementIndex));
		const FMaterialElement& MaterialElement = MaterialElements(ElementIndex);
		const FLOAT MaskClipValue = MaterialElement.Material->OpacityMaskClipValue;
		const FLOAT OpacityMask = MaterialElement.Material->SampleTransmission(UVs).R;
		return OpacityMask > MaskClipValue;
	}

	/** Evaluates the mesh's tangent space normal at the given UVs */
	inline FVector4 EvaluateNormal(const FVector2D& UVs, INT ElementIndex) const
	{
		FVector4 Normal( 0, 0, 1.0f, 0.0 );
		const FMaterialElement& MaterialElement = MaterialElements(ElementIndex);
		if( MaterialElement.Material->NormalSize > 0 )
		{
			MaterialElement.Material->SampleNormal(UVs, Normal);
		}
		return Normal;
	}

	/** 
	 * Returns the hemispherical-hemispherical reflectance, 
	 * Which is the fraction of light that is reflected in any direction when the incident light is constant over all directions of the hemisphere.
	 * This value is used to calculate exitant radiance, which is 1 / PI * HemisphericalHemisphericalReflectance * Irradiance, disregarding directional variation.
     */
	inline FLinearColor EvaluateTotalReflectance(const FStaticLightingVertex& Vertex, INT ElementIndex) const
	{
		return EvaluateDiffuse(Vertex.TextureCoordinates[0], ElementIndex);
	}

	virtual void Read( FILE* FileHandle, FScene& Scene);
	virtual void Import( class FLightmassImporter& Importer );

	/** Allows the mesh to create mesh area lights from its emissive contribution */
	void CreateMeshAreaLights(const class FStaticLightingSystem& LightingSystem, const FScene& Scene, TIndirectArray<FMeshAreaLight>& MeshAreaLights) const;

private:

	/** Splits a mesh into layers with non-overlapping UVs, maintaining adjacency in world space and UVs. */
	void CalculateUniqueLayers(const TArray<FStaticLightingVertex>& MeshVertices, const TArray<INT>& ElementIndices, TArray<TArray<INT> >& LayeredGroupTriangles) const;

	/** Adds an entry to Texels if the given texel passes the emissive criteria. */
	void AddLightTexel(
		const class FTexelToCornersMap& TexelToCornersMap, 
		INT ElementIndex,
		TArray<INT>& LightIndices, 
		INT X, INT Y, 
		FLOAT EmissiveThreshold,
		TArray<FIntPoint>& Texels,
		INT TexSizeX,
		INT TexSizeY) const;

	/** Adds an entry to Texels if the given texel passes the primitive simplifying criteria. */
	void AddPrimitiveTexel(
		const FTexelToCornersMap& TexelToCornersMap, 
		const struct FTexelToCorners& ComparisonTexel,
		INT ComparisonTexelLightIndex,
		const FVector4& PrimitiveOrigin,
		TArray<INT>& PrimitiveIndices, 
		const TArray<INT>& LightIndices, 
		INT X, INT Y, 
		TArray<FIntPoint>& Texels,
		const FScene& Scene,
		FLOAT DistanceThreshold) const;
};


} //namespace Lightmass
