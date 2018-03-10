/*=============================================================================
	SceneExport.h: Scene export data definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "MeshExport.h"

namespace Lightmass
{

#pragma pack(push, 1)

static const INT NumTexelCorners = 4;

/** 
 * General and misc solver settings.
 * Settings prefixed by Debugging are only useful for development, 
 * Either because one of the options have been proven superior or because it produces a visualization of an intermediate result.
 * All angles are in Radians, distances are in world space units.
 */
class FStaticLightingSettings 
{
public:
	/** Debugging - whether to allow multiple static lighting threads. */
	UBOOL bAllowMultiThreadedStaticLighting;

	/**
	 * Number of local cores to leave unused
	 */
	INT NumUnusedLocalCores;

	/** 
	 * Number of indirect lighting bounces to simulate, 0 is direct lighting only. 
	 * The first bounce always costs the most in terms of computation time, with the second bounce following.  
	 * With photon mapping, bounces after the second are nearly free.
	 */
	INT NumIndirectLightingBounces;

	/** 
	 * Debugging - which single light bounce to view or -1 for all.  
	 * This setting has been carefully implemented to only affect the final color and not affect any other part of the solver (sample positions, ray directions, etc).
	 * Most of the debug visualizations are affected by ViewSingleBounceNumber.
	 */
	INT ViewSingleBounceNumber;

	/** 
	 * Debugging - when enabled, multiple samples will be used to detect all the texels that are mapped to geometry. 
	 * Otherwise only the center and corner of each texel will be sampled. 
	 */
	UBOOL bUseConservativeTexelRasterization;

	/** Debugging - whether to use the texel size in various calculations in an attempt to compensate for point sampling a texel. */
	UBOOL bAccountForTexelSize;

	/** Debugging - whether to use the sample with the largest weight when rasterizing texels or a linear combination. */
	UBOOL bUseMaxWeight;

	/** Maximum lighting samples per triangle for vertex lightmaps. */
	INT MaxTriangleLightingSamples;

	/** Maximum samples for caching irradiance photons per triangle for vertex lightmaps. */
	INT MaxTriangleIrradiancePhotonCacheSamples;

	/** Debugging - whether to color texels when invalid settings are detected. */
	UBOOL bUseErrorColoring;

	/** Unmapped texel color */
	FLinearColor UnmappedTexelColor;
};

/** Scale dependent constants */
class FStaticLightingSceneConstants
{
public:
	/** 
	 * Scale of the level being lit relative to Gears of War 2 levels.  
	 * All global, scale dependent defaults are tweaked for Gears2 levels, 
	 * Games using a different scale should use this to convert the defaults into the game-specific scale.
	 */
	FLOAT StaticLightingLevelScale;

	/** 
	 * World space distance to offset the origin of the ray, along the direction of the ray. 
	 * This is used to prevent incorrect self shadowing due to floating point precision.
	 */
	FLOAT VisibilityRayOffsetDistance;

	/** 
	 * World space distance to offset the origin of the ray along the direction of the normal.
	 * This is used to push triangle shaped self shadowing artifacts onto the backfaces of curved objects.
	 */
	FLOAT VisibilityNormalOffsetDistance;

	/** 
	 * Fraction of the sample radius to offset the origin of the ray along the sample normal. 
	 * This is applied instead of VisibilityNormalOffsetDistance whenever sample radius is known as it adapts to differently sized texels.
	 */
	FLOAT VisibilityNormalOffsetSampleRadiusScale;

	/** 
	 * Fraction of the sample radius to offset the origin of the ray in the tangent XY plane, based on the direction of the ray.
	 * This is only used when bAccountForTexelSize is TRUE.
	 */
	FLOAT VisibilityTangentOffsetSampleRadiusScale;

	/** 
	 * Smallest texel radius allowed, useful for clamping edge cases where some texels have a radius of 0. 
	 * This should be smaller than the smallest valid texel radius in the scene.
	 */
	FLOAT SmallestTexelRadius;

	/** 
	 * Size of the grid that each light will use to cache information.  
	 * Larger grids take longer to precompute, but result in accelerated light sampling.
	 */
	INT LightGridSize;

	/** Power to apply to dot(N,L) before scaling direct lighting stored in directional coefficients. */
	FLOAT DirectionalCoefficientFalloffPower;

	/** Scale to apply to direct lighting stored in directional coefficients. */
	FLOAT DirectionalCoefficientScale;

	/** 
	 * Lerp factor that controls the influence of normal maps with directional lightmaps on indirect lighting.
	 * A value of 0 gives a physically correct distribution of light, which may result in little normal influence in areas only lit by indirect lighting, but less lightmap compression artifacts.
	 * A value of .8 results in 80% of the lighting being redistributed in the dominant incident lighting direction, which effectively increases the per-pixel normal's influence,
	 * But causes more severe lightmap compression artifacts.
	 */
	FLOAT IndirectNormalInfluenceBoost;

	/**
	 * Settings for an optional fixed scale, rather than the general one computed at Quantize time, applied to simple lightmaps to allow for enhanced
	 * range for over-brightening while improving runtime performance for lower end platforms and mobile. 
	 */
	UBOOL bUseFixedScaleForSimpleLightmaps;
	FLOAT UseFixedScaleValue;
};

/** Settings for which material attribute to visualize */
enum EViewMaterialAttribute
{
	VMA_None,
	VMA_Emissive,
	VMA_Diffuse,
	VMA_Specular,
	VMA_SpecularPower,
	VMA_Transmission,
	VMA_Normal
};

/** The scenes material settings */
struct FSceneMaterialSettings
{
	/** Debugging - Whether to use the debug material */
	UBOOL bUseDebugMaterial;

	/** Debugging - Indicates which material attribute to visualize. */
	EViewMaterialAttribute ViewMaterialAttribute;

	/** The size of the emissive sample */
	INT EmissiveSize;

	/** The size of the diffuse sample */
	INT DiffuseSize;

	/** The size of the Specular sample */
	INT SpecularSize;

	/** The size of the Transmission sample */
	INT TransmissionSize;

	/** The size of the normal sample */
	INT NormalSize;


	/** Enables normal map sampling when Lightmass is generating 'simple' light maps.  This increases lighting build time, but may improve quality when normal maps are used to represent curvature over a large surface area.  When this setting is disabled, 'simple' light maps will not take normal maps into account. */
	/** Note: This setting is driven from the level's UEngine::bUseNormalMapsForSimpleLightMaps option */
	UBOOL bUseNormalMapsForSimpleLightMaps;


	/** 
	 * Debugging - Amount of incoming light to reflect diffusely (equally in all directions). 
	 * This is the diffuse term in the modified Phong BRDF, not the original.
	 */
	FLinearColor DebugDiffuse;

	/** 
	 * Debugging - Amount of incoming light to reflect along the reflected incoming vector. 
	 * This is the specular term in the modified Phong BRDF, not the original.
	 * For physically valid materials that obey conservation of energy, all components of Specular + Diffuse are <= 1.
	 */
	FLinearColor DebugSpecular;

	/** 
	 * Debugging - Power applied to the modified Phong specular cosine lobe.  
	 * Higher powers result in sharper reflections, which also brightens the specular lobe due to modified Phong weighting.
	 */
	FLOAT DebugSpecularPower;

	/** Debugging - Emissive value assigned to secondary rays which miss all geometry in the scene. */
	FLinearColor EnvironmentColor;

	/**
	 *	In advanced mode the color that rays which miss the scene will pick up depends on 'sun'.
	 *  The closer ray's direction is to the direction of sun the more color of sun is blended/added into picked color.
	 */
	UBOOL bEnableAdvancedEnvironmentColor;
	/** Color of the sun */
	FLinearColor EnvironmentSunColor;

	/** Angle of sun color - factor */
	FLOAT EnvironmentLightTerminatorAngleFactor;

	/** Direction of sun - usually it's direction of dominant light */
	FVector4 EnvironmentLightDirection;
};

/** Settings for meshes which emit light from their emissive areas. */
struct FMeshAreaLightSettings
{
	/** Whether to draw debug lines showing the corners of mesh area light primitives when a texel has been selected. */
	UBOOL bVisualizeMeshAreaLightPrimitives;

	/** Emissive intensities must be larger than this to contribute toward scene lighting. */
	FLOAT EmissiveIntensityThreshold;

	/** 
	* Size of the grid that each mesh area light will use to cache information.  
	* Larger grids take longer to precompute, but result in accelerated light sampling.
	*/
	INT MeshAreaLightGridSize;

	/** Cosine of the maximum angle allowed between mesh area light primitives that get merged into the same simplified primitive. */
	FLOAT MeshAreaLightSimplifyNormalCosAngleThreshold;

	/** Maximum distance allowed between any mesh area light primitive corners that get merged into the same simplified primitive. */
	FLOAT MeshAreaLightSimplifyCornerDistanceThreshold;

	/** Fraction of a mesh's bounds that an emissive texel can be from a simplified primitive and still get merged into that primitive. */
	FLOAT MeshAreaLightSimplifyMeshBoundingRadiusFractionThreshold;

	/** Distance along the average normal from the bounds origin of the mesh area light to place a light to handle influencing dynamic objects. */
	FLOAT MeshAreaLightGeneratedDynamicLightSurfaceOffset;
};

/** AO settings */
class FAmbientOcclusionSettings
{
public:
	/** 
	 * Whether to calculate ambient occlusion. 
	 * When enabled, some final gather rays will be traced even if only direct lighting is being calculated.
	 */
	UBOOL bUseAmbientOcclusion;

	/** Debugging - whether to only show the ambient occlusion term, useful for seeing the impact of AO settings in isolation. */
	UBOOL bVisualizeAmbientOcclusion;

	/** 
	 * How much of the ambient occlusion term should be applied to direct lighting.
	 * A value of 0 will leave the direct lighting untouched, a value of 1 will apply all of the occlusion.
	 */
	FLOAT DirectIlluminationOcclusionFraction;
	
	/** Same as DirectIlluminationOcclusionFraction, but applied to indirect lighting. */
	FLOAT IndirectIlluminationOcclusionFraction;
	
	/** 
	 * Controls the ambient occlusion contrast.  
	 * Higher powers result in more contrast, which effectively shorten the occlusion gradient and push it into corners.
	 * An exponent of 1 gives equivalent occlusion as the indirect shadows from a constant environment color, with no other lights in the scene.
	 */
	FLOAT OcclusionExponent;
	
	/** 
	 * Fraction of the samples taken that have to be occluded before an occlusion value of 1 is reached for that texel. 
	 * A value of 1 means all the samples in the hemisphere have to be occluded, lower values darken areas that are not fully occluded.
	 */
	FLOAT FullyOccludedSamplesFraction;
	
	/** 
	 * Maximum distance over which an object can affect the occlusion of a texel.
	 * Lowering this cuts off lower frequencies in the ambient occlusion.
	 */
	FLOAT MaxOcclusionDistance;
};

/** Settings related to precomputations used by dynamic objects. */
class FDynamicObjectSettings
{
public:
	/** Debugging - Whether to draw points in the editor to visualize volume lighting samples. */
	UBOOL bVisualizeVolumeLightSamples;

	/** Debugging - Whether to interpolate indirect lighting for surfaces from the precomputed light volume. */
	UBOOL bVisualizeVolumeLightInterpolation;

	/** World space distance between samples placed on upward facing surfaces. */
	FLOAT SurfaceLightSampleSpacing;
	/** Height of the first sample layer above the surface. */
	FLOAT FirstSurfaceSampleLayerHeight;
	/** Height difference of successive layers. */
	FLOAT SurfaceSampleLayerHeightSpacing;
	/** Number of layers to place above surfaces. */
	INT NumSurfaceSampleLayers;
	/** Distance between samples placed in a 3d uniform grid inside detail volumes. */ 
	FLOAT DetailVolumeSampleSpacing;
	/** Distance between samples placed in a 3d uniform grid inside the importance volume. */
	FLOAT VolumeLightSampleSpacing;
	/** Maximum samples placed in the 3d volume, used to limit memory usage. */
	INT MaxVolumeSamples;
	/** Use Maximum number restriction for Surface Light Sample. */
	UBOOL bUseMaxSurfaceSampleNum;
	/** Maximum samples placed in the surface light sample(only for Landscape currently), used to limit memory usage. */
	INT MaxSurfaceLightSamples;
};

/** Settings for precomputed visibility. */
class FPrecomputedVisibilitySettings
{
public:

	/** Whether to export debug lines for visibility. */
	UBOOL bVisualizePrecomputedVisibility;

	/** Whether to only place visibility cells on opaque surfaces. */
	UBOOL bPlaceCellsOnOpaqueOnly;

	/** Whether to place visibility cells on shadow casting surfaces only, or everywhere inside Precomputed Visibility Volumes. */
	UBOOL bPlaceCellsOnSurfaces;

	/** World space size of visibility cells in the x and y dimensions. */
	FLOAT CellSize;

	/** Number of tasks that visibility cells are being split up into. */
	INT NumCellDistributionBuckets;

	/** World space size of visibility cells in the z dimension. */
	FLOAT PlayAreaHeight;

	/** Amount to increase the bounds of meshes when querying their visibility.  Larger scales reduce visibility errors at the cost of less effective culling. */
	FLOAT MeshBoundsScale;

	/** Minimum number of samples on the mesh for each cell - mesh query.  Small meshes use less samples. */
	INT MinMeshSamples;

	/** Maximum number of samples on the mesh for each cell - mesh query.  Small meshes use less samples. */
	INT MaxMeshSamples;

	/** Number of samples on each cell for each cell - mesh query. */
	INT NumCellSamples;

	/** Number of samples to use when importance sampling each cell - mesh query. */
	INT NumImportanceSamples;
};

/** Settings for volume distance field generation. */
class FVolumeDistanceFieldSettings
{
public:

	/** World space size of a voxel.  Smaller values use significantly more memory and processing time, but allow more detailed shadows. */
	FLOAT VoxelSize;

	/** 
	 * Maximum world space distance represented in the distance field, used for normalization. 
	 * Larger values increase build time and decrease distance field precision, but allow distance field traversal to skip larger areas. 
	 */
	FLOAT VolumeMaxDistance;

	 /** Number of distance traces for each voxel. */
	INT NumVoxelDistanceSamples;

	/** Upper limit on the number of voxels that can be generated. */
	INT MaxVoxels;
};

/** Shadow settings */
class FStaticShadowSettings
{
public:
	/** Debugging - Whether to filter a single shadow sample per texel in texture space instead of calculating area shadows. */
	UBOOL bUseZeroAreaLightmapSpaceFilteredLights;

	/** Number of shadow rays to trace to each area light for each texel. */
	INT NumShadowRays;

	/** Number of shadow rays to trace to each area light once a texel has been determined to lie in a shadow penumbra. */
	INT NumPenumbraShadowRays;

	/** 
	 * Number of shadow rays to trace to each area light for bounced lighting. 
	 * This number is divided by bounce number for successive bounces.
	 */
	INT NumBounceShadowRays;

	/**
	 * Settings for enabling and adjusting a filter pass on the computed shadow factor. The shadow factor is
	 * a value [0,1] that approximates the percentage of an area light visible at a texel. A value of 0 indicates
	 * that the texel is completely in shadow and 1 indicates that it is completely lit. The tolerance value is the
	 * maximum difference in the absolute shadow factor value allowed between any two adjacent texels before enabling
	 * filtering, meant to catch shadow transitions sharper than can be captured in the lightmap.
	 */
	UBOOL bFilterShadowFactor;
	FLOAT ShadowFactorGradientTolerance;

	/** Whether to allow signed distance field shadows, or fall back on area shadows. */
	UBOOL bAllowSignedDistanceFieldShadows;

	/** 
	 * Maximum world space distance stored from a texel to the shadow transition.
	 * Larger distances decrease precision but increase the maximum penumbra size that can be reconstructed from the distance field.
	 */
	FLOAT MaxTransitionDistanceWorldSpace;

	/** 
	 * The number of high resolution samples to calculate per MaxTransitionDistanceWorldSpace. 
	 * Higher values increase the distance field reconstruction quality, at the cost of longer build times.
	 */
	INT ApproximateHighResTexelsPerMaxTransitionDistance;

	/** 
	 * The minimum upsample factor to calculate the high resolution samples at.  
	 * Larger values increase distance field reconstruction quality on small, high resolution meshes, at the cost of longer build times.
	 */
	INT MinDistanceFieldUpsampleFactor;

	/** Distance in world space units between dominant light shadow map cells. */
	FLOAT DominantShadowTransitionSampleDistanceX;
	FLOAT DominantShadowTransitionSampleDistanceY;

	/** Amount to super sample dominant shadow map generation, in each dimension.  Larger factors increase build time but produce a more conservative shadow map. */
	INT DominantShadowSuperSampleFactor;

	/** Maximum number of dominant shadow samples to generate for one dominant light, used to limit memory used. */
	INT DominantShadowMaxSamples;

	/** Fraction of valid lighting samples (mapped texels or vertex samples) that must be unoccluded in a precomputed shadowmap for the shadowmap to be kept. */
	FLOAT MinUnoccludedFraction;
};

/** 
 * Settings related to solving the light transport equation by starting from the source of importance, 
 * as opposed to starting from the source of radiant power. 
 */
class FImportanceTracingSettings
{
public:
	/** 
	 * Debugging - whether to use path tracing to calculate indirect lighting. 
	 * Path tracing does not spawn new rays at each light bounce but instead follows the same path up until the last bounce.
	 * More paths are required to compensate for noise in the later bounces.  Only the first bounce is cached with the irradiance cache.
	 * This is overridden if bUsePhotonMapping is TRUE.
	 */
	UBOOL bUsePathTracer;

	/** 
	 * Debugging - whether to use a cosine probability distribution function when generating hemisphere samples.   
	 * This is only usable with irradiance caching off.
	 */
	UBOOL bUseCosinePDF;

	/** 
	 * Debugging - whether to stratify hemisphere samples, which reduces variance. 
	 * This is not supported with bUseCosinePDF being TRUE and is required when bUseIrradianceGradients is TRUE.
	 */
	UBOOL bUseStratifiedSampling;

	/** Debugging - number of paths to trace from each irradiance cache sample when bUsePathTracer is TRUE */
	INT NumPaths;

	/** 
	 * Number of hemisphere samples to evaluate from each irradiance cache sample when not using path tracing.
	 * When photon mapping is enabled, these are called final gather rays.
	 */
	INT NumHemisphereSamples;

	/** 
	 * Debugging - number of hemisphere samples to evaluate for second bounce lighting. 
	 * This is divided by bounce number for each successive bounce, and is not used if photon mapping is enabled, 
	 * Because light interactions after the first bounce are estimated using the photon maps.
	 */
	INT NumBounceHemisphereSamples;

	/** 
	 * Largest angle from the sample normal that a hemisphere sample direction can be. 
	 * Useful for preventing rays nearly perpendicular to the normal which may self intersect a flat surface due to imprecision in the normal.
	 */
	FLOAT MaxHemisphereRayAngle;
};

/** Settings controlling photon mapping behavior. */
class FPhotonMappingSettings
{
public:
	/** 
	 * Debugging - whether to use photon mapping.  
	 * Photon mapping benefits many parts of the solver so this is mainly for comparing against other methods.
	 */
	UBOOL bUsePhotonMapping;

	/** 
	 * Debugging - whether to estimate the first bounce lighting by tracing rays from the sample point,
	 * Which is called final gathering, or by using the density of nearby first bounce photons.  
	 * Final gathering is slow but gets vastly better results than using first bounce photons.
	 */
	UBOOL bUseFinalGathering;

	/** 
	* Debugging - whether to use direct photon density as an estimate for direct lighting. 
	* This is mainly useful for debugging what the final gather ray see.
	*/
	UBOOL bUsePhotonsForDirectLighting;

	/** Debugging - whether to use the optimization of caching irradiance calculations in deposited photons (called Irradiance photons). */
	UBOOL bUseIrradiancePhotons;

	/** 
	* Debugging - whether to cache the result of the search for the nearest irradiance photon on surfaces. 
	* This results in a constant time lookup at the end of each final gather ray instead of a photon map search.
	* Only visible photons are cached, which reduces light leaking.
	*/
	UBOOL bCacheIrradiancePhotonsOnSurfaces;

	/** 
	 * Whether to use direct photons to determine which texels need shadow rays traced.
	 * This can reduce direct lighting time by a decent amount but requires lots of direct photons to avoid masking artifacts.
	 */
	UBOOL bOptimizeDirectLightingWithPhotons;

	/** 
	 * Debugging - whether to draw lines in the editor representing photon paths.  
	 * They will only be drawn if a texel is selected and ViewSingleBounceNumber is >= 0.
	 */
	UBOOL bVisualizePhotonPaths;

	/** 
	 * Debugging - whether to draw the photons gathered for the selected texel or the photons gathered by final gather rays from the selected texel.
	 * This includes caustic, importance, irradiance and direct photons.
	 */
	UBOOL bVisualizePhotonGathers;

	/** 
	 * Debugging - whether to draw lines in the editor showing where rays were traced due to importance photons from the selected texel.  
	 * If this is FALSE, lines will be drawn for the rays traced with uniform hemisphere sampling instead.
	 * Note that when irradiance caching is enabled, rays will only be gathered if the selected texel created an irradiance cache sample.
	 */
	UBOOL bVisualizePhotonImportanceSamples;

	/** 
	 * Debugging - whether to gather and draw photon map octree nodes that get traversed during the irradiance calculation for the irradiance photon nearest to the select position.
	 * Which photon map search the nodes come from depends on ViewSingleBounceNumber.
	 */
	UBOOL bVisualizeIrradiancePhotonCalculation;

	/** 
	 * Debugging - whether to emit any photons outside the importance volume, if one exists.  
	 * If this is FALSE, nothing outside the volume will bounce lighting and will be lit with direct lighting only.
	 */
	UBOOL bEmitPhotonsOutsideImportanceVolume;

	/** Cone filter constant, which characterizes the falloff of the filter applied to photon density estimations. */
	FLOAT ConeFilterConstant;

	/** Number of photons to find in each photon map when calculating irradiance for an irradiance photon. */
	INT NumIrradianceCalculationPhotons;

	/** 
	 * Fraction of NumHemisphereSamples to use for importance sampling instead of uniform sampling the final gather.
	 * If this fraction is close to 1, no uniform samples will be done and irradiance caching will be forced off as a result.
	 * If this is 0, only uniform samples will be taken.
	 */
	FLOAT FinalGatherImportanceSampleFraction;

	/** Cosine of the cone angle from an importance photon direction to generate ray directions for importance sampled final gathering. */
	FLOAT FinalGatherImportanceSampleCosConeAngle;

	/** World space radius of the disk around an indirect photon path in which indirect photons will be emitted from directional lights. */
	FLOAT IndirectPhotonEmitDiskRadius;

	/** Angle around an indirect photon path in which indirect photons will be emitted from point, spot and mesh area lights. */
	FLOAT IndirectPhotonEmitConeAngle;

	/** Maximum distance to search for importance photons. */
	FLOAT MaxImportancePhotonSearchDistance;

	/** 
	 * Distance to start searching for importance photons at. 
	 * For some scenes a small start distance (relative to MaxImportancePhotonSearchDistance) will speed up the search
	 * Since it can early out once enough photons are found.  For other scenes a small start distance will just cause redundant photon map searches.
	 */
	FLOAT MinImportancePhotonSearchDistance;

	/** Number of importance photons to find at each irradiance cache sample that will be used to guide the final gather. */
	INT NumImportanceSearchPhotons;

	/** Scales the density at which to gather photons outside of the importance volume, if one exists. */
	FLOAT OutsideImportanceVolumeDensityScale;

	/** 
	 * Density of direct photons to emit per light, in number of photons per million surface area units.
	 * This should be fairly high if bOptimizeDirectLightingWithPhotons is TRUE.
	 */
	FLOAT DirectPhotonDensity;

	/** Density of direct photons which have irradiance cached at their position, in number of photons per million surface area units. */
	FLOAT DirectIrradiancePhotonDensity;

	/** Distance to use when searching for direct photons. */
	FLOAT DirectPhotonSearchDistance;

	/**
	 * Target density of indirect photon paths to gather, in number of paths per million surface area units. 
	 * Densities too small will result in indirect lighting not making it into small pockets.
	 */
	FLOAT IndirectPhotonPathDensity;

	/** 
	 * Density of indirect photons to emit, in number of photons per million surface area units.
	 * This should be high because first bounce photons are used to guide the final gather.
	 */
	FLOAT IndirectPhotonDensity;

	/** Density of indirect photons which have irradiance cached at their position, in number of photons per million surface area units. */
	FLOAT IndirectIrradiancePhotonDensity;

	/** Distance to use when searching for indirect photons. */
	FLOAT IndirectPhotonSearchDistance;

	/** 
	 * Target density of caustic photons, in number of photons per million surface area units.
	 * This should be very high for scenes with lots of high frequency specular materials.
	 */
	FLOAT CausticPhotonDensity;

	/** Distance to search for caustic photons */
	FLOAT CausticPhotonSearchDistance;

	/** Maximum cosine of the angle between the search normal and the surface normal of a candidate photon for that photon to be a valid search result. */
	FLOAT PhotonSearchAngleThreshold;

	/** Cosine of the angle from the search normal that defines a cone which irradiance photons must be outside of to be valid for that search. */
	FLOAT MinCosIrradiancePhotonSearchCone;

	/** 
	 * Cosine of the maximum angle between the light direction and a surface's normal at which to allow the direct lighting photon optimization.
	 * The probability of receiving direct photons decreases as this angle increases, 
	 * So this cutoff prevents the optimization from being applied to surfaces at glancing angles to the light.
	 */
	FLOAT MaxCosDirectLightingPhotonOptimizationAngle;

	/** Downsample factor applied to each mapping's lighting resolution to get the resolution used for caching irradiance photons. */
	FLOAT CachedIrradiancePhotonDownsampleFactor;

	/** Determines how far away a photon can be to be reused by a point outside the importance volume. */
	FLOAT OutsideVolumeIrradiancePhotonDistanceThreshold;
};

/** Settings controlling irradiance caching behavior. */
class FIrradianceCachingSettings
{
public:
	/** Debugging - whether to allow irradiance caching.  When this is disabled, indirect lighting will be many times slower. */
	UBOOL bAllowIrradianceCaching;

	/** Debugging - whether to use irradiance gradients, which effectively allows higher order irradiance cache interpolation. */
	UBOOL bUseIrradianceGradients;

	/** Debugging - whether to only show irradiance gradients. */
	UBOOL bShowGradientsOnly;

	/** Debugging - whether to draw debug elements in the editor showing which irradiance cache samples were used to shade the selected texel. */
	UBOOL bVisualizeIrradianceSamples;

	/** Scale applied to the radius of irradiance cache records.  This directly affects sample placement and therefore quality. */
	FLOAT RecordRadiusScale;

	/** Maximum angle between a record and the point being shaded allowed for the record to contribute. */
	FLOAT InterpolationMaxAngle;

	/** 
	 * Maximum angle from the plane defined by the average normal of a record and the point being shaded 
	 * That the vector from the point to the record can be for that record to contribute. 
	 */
	FLOAT PointBehindRecordMaxAngle;

	/** 
	 * How much to increase RecordRadiusScale for the shading pass. 
	 * This effectively filters the irradiance on flat surfaces.
	 */
	FLOAT DistanceSmoothFactor;

	/** 
	 * How much to increase InterpolationMaxAngle for the shading pass. 
	 * This effectively filters the irradiance on curved surfaces.
	 */
	FLOAT AngleSmoothFactor;

	/** Same as RecordRadiusScale but used for bounces after the first when hemisphere gathering is being used alone (without photon mapping and path tracing). */
	FLOAT RecordBounceRadiusScale;

	/** 
	 * Smallest radius an irradiance cache record can have. 
	 * This has a large impact on performance since a small value will allow a lot of records clumped in corners, while a large value will reduce quality in corners.
	 */
	FLOAT MinRecordRadius;

	/** Largest radius an irradiance cache record can have. */
	FLOAT MaxRecordRadius;
};

struct FDebugLightingInputData
{
	/** Whether the solver should send stats back to UE3 */
	UBOOL bRelaySolverStats;
	/** Guid of the mapping to debug */
	FGuid MappingGuid;
	/** Index of the BSP node to debug if the mapping is a BSP mapping */
	INT NodeIndex;
	/** World space position of the position that was clicked to select the debug sample */
	FVector4 Position;
	/** Position in the texture mapping of the texel to debug */
	INT LocalX;
	INT LocalY;
	/** Size of the texture mapping that was selected to debug */
	INT MappingSizeX;
	INT MappingSizeY;
	/** Position in the lightmap atlas of the texel to debug */
	INT LightmapX;
	INT LightmapY;
	/** Index of the vertex being debugged, if the mapping is using vertex lightmaps. */
	INT VertexIndex;
	/** Color in the lightmap atlas before the sample was selected */
	FColor OriginalColor;
	/** Position of the camera */
	FVector4 CameraPosition;
	/** VisibilityId of a component from the currently selected actor or BSP surface. */
	INT DebugVisibilityId;
};

//----------------------------------------------------------------------------
//	Scene export file header
//----------------------------------------------------------------------------
struct FSceneFileHeader
{
	/** FourCC cookie: 'SCEN' */
	UINT		Cookie;
	FGuid		FormatVersion;
	FGuid		Guid;

	// 标志是第几张光照图
	INT		Num;

	/** Settings for the GI solver */
	FStaticLightingSettings			GeneralSettings;
	FStaticLightingSceneConstants	SceneConstants;
	FSceneMaterialSettings			MaterialSettings;
	FMeshAreaLightSettings			MeshAreaLightSettings;
	FAmbientOcclusionSettings		AmbientOcclusionSettings;
	FDynamicObjectSettings			DynamicObjectSettings;
	FPrecomputedVisibilitySettings	PrecomputedVisibilitySettings;
	FVolumeDistanceFieldSettings	VolumeDistanceFieldSettings;
	FStaticShadowSettings			ShadowSettings;
	FImportanceTracingSettings		ImportanceTracingSettings;
	FPhotonMappingSettings			PhotonMappingSettings;
	FIrradianceCachingSettings		IrradianceCachingSettings;
	
	FDebugLightingInputData			DebugInput;

	FSceneFileHeader() {}

	/** Copy ctor that doesn't modify padding in FSceneFileHeader. */
	FSceneFileHeader(const FSceneFileHeader& Other);

	void SetCustomImportanceBoundingBox(const FBox& NewBox) { CustomImportanceBoundingBox = NewBox; }

protected:

	/** 
	 *	The bounding box of the 'custom' LightmassImportance volume.
	 *	If this is non-zero, it indicates that the build is for selected
	 *	actors only. In this case, the solver will use this importance
	 *	volume but trace rays outside of it to find light sources.
	 */
	FBox CustomImportanceBoundingBox;

public:

	/** If TRUE, pad the mappings (shrink the requested size and then pad) */
	BITFIELD	bPadMappings:1;
	/** If TRUE, draw a solid border as the padding around mappings */
	BITFIELD	bDebugPadding:1;
	/** If TRUE, only calculate lighting on debugged texel's mappings */
	BITFIELD	bOnlyCalcDebugTexelMappings:1;
	/** If TRUE, color lightmaps based on execution time (brighter red = slower) */
	BITFIELD	bColorByExecutionTime:1;
	/** If TRUE, color lightmaps a random color */
	BITFIELD	bUseRandomColors:1;
	/** If TRUE, a green border will be placed around the edges of mappings */
	BITFIELD	bColorBordersGreen:1;

	/** WITH_SPEEDTREE flag */
	BITFIELD	bWithSpeedTree:1;

	BITFIELD	bBuildOnlyVisibleLevels:1;

	/** Amount of time to color full red (Color.R = Time / ExecutionTimeDivisor) */
	FLOAT		ExecutionTimeDivisor;

	INT		NumImportanceVolumes;
	INT		NumCharacterIndirectDetailVolumes;
	INT		NumDirectionalLights;
	INT		NumPointLights;
	INT		NumSpotLights;
	INT		NumSkyLights;
	INT		NumStaticMeshes;
	INT		NumTerrains;
	INT		NumStaticMeshInstances;
	INT		NumFluidSurfaceInstances;
	INT		NumLandscapeInstances;
	INT		NumSpeedTreeLightingMeshes;
	INT		NumBSPMappings;
	INT		NumStaticMeshTextureMappings;
	INT		NumStaticMeshVertexMappings;
	INT		NumTerrainMappings;
	INT		NumFluidSurfaceTextureMappings;
	INT		NumLandscapeTextureMappings;
	INT		NumSpeedTreeMappings;
	INT		NumPrecomputedVisibilityBuckets;
};

//----------------------------------------------------------------------------
//	Base light struct
//----------------------------------------------------------------------------
enum EDawnLightFlags
{
	// maps to ULightComponent::CastShadows
	GI_LIGHT_CASTSHADOWS			= 0x00000001,
	// maps to ULightComponent::HasStaticLighting()
	GI_LIGHT_HASSTATICLIGHTING		= 0x00000002,
	// maps to ULightComponent::UseDirectLightMap
	GI_LIGHT_USEDIRECTLIGHTMAP		= 0x00000004,
	// maps to ULightComponent::HasStaticShadowing()
	GI_LIGHT_HASSTATICSHADOWING		= 0x00000008,
	// maps to ULightComponent::CastStaticShadows
	GI_LIGHT_CASTSTATICSHADOWS		= 0x00000010,
	GI_LIGHT_USESIGNEDDISTANCEFIELDSHADOWS = 0x00000020,
	GI_LIGHT_DOMINANT				= 0x00000040
};

struct FLightData
{
	FGuid		Guid;
	/** Bit-wise combination of flags from EDawnLightFlags */
	UINT		LightFlags;
	/** Homogeneous coordinates */
	FVector4	Position;
	FVector4	Direction;
	FColor		Color;
	FLOAT		Brightness;
	/** The radius of the light's surface, not the light's influence. */
	FLOAT		LightSourceRadius;
	/** Scale factor for the indirect lighting */
	FLOAT		IndirectLightingScale;
	/** 0 will be completely desaturated, 1 will be unchanged, 2 will be completely saturated */
	FLOAT		IndirectLightingSaturation;
	/** Controls the falloff of shadow penumbras */
	FLOAT		ShadowExponent;
};

//----------------------------------------------------------------------------
//	Direction light, extending FLightData
//----------------------------------------------------------------------------
struct FDirectionalLightData
{
	/** Angle that the directional light's emissive surface extends from any receiver position, in radians. */
	FLOAT		LightSourceAngle;
};

//----------------------------------------------------------------------------
//	Point light, extending FLightData
//----------------------------------------------------------------------------
struct FPointLightData
{
	FLOAT		Radius;
	FLOAT		FalloffExponent;
};

//----------------------------------------------------------------------------
//	Spot light, extending FPointLightData
//----------------------------------------------------------------------------
struct FSpotLightData
{
	FLOAT		InnerConeAngle;
	FLOAT		OuterConeAngle;
};

//----------------------------------------------------------------------------
//	Sky light, extending FLightData
//----------------------------------------------------------------------------
struct FSkyLightData
{
	FLOAT		LowerBrightness;
	FColor		LowerColor;
};

//----------------------------------------------------------------------------
//	Material
//----------------------------------------------------------------------------

struct FMaterialElementData
{
	/** Used to find Material on import */
	FGuid MaterialId;
	/** If TRUE, this object will be lit as if it receives light from both sides of its polygons. */
	BITFIELD bUseTwoSidedLighting:1;
	/** If TRUE, this material element will only shadow indirect lighting.  					*/
	BITFIELD bShadowIndirectOnly:1;
	/** If TRUE, allow using the emissive for static lighting.						*/
	BITFIELD bUseEmissiveForStaticLighting:1;
	/** Direct lighting falloff exponent for mesh area lights created from emissive areas on this primitive. */
	FLOAT EmissiveLightFalloffExponent;
	/** 
	 * Direct lighting influence radius.  
	 * The default is 0, which means the influence radius should be automatically generated based on the emissive light brightness.
	 * Values greater than 0 override the automatic method.
	 */
	FLOAT EmissiveLightExplicitInfluenceRadius;
	/** Scales the emissive contribution of this material. */
	FLOAT EmissiveBoost;
	/** Scales the diffuse contribution of this material. */
	FLOAT DiffuseBoost;
	/** Scales the specular contribution of this material. */
	FLOAT SpecularBoost;
	/** 
	 * Fraction of the samples taken that have to be occluded before an occlusion value of 1 is reached for that texel. 
	 * A value of 1 means all the samples in the hemisphere have to be occluded, lower values darken areas that are not fully occluded.
	 */
	FLOAT FullyOccludedSamplesFraction;

	FMaterialElementData() :
		  bUseTwoSidedLighting(FALSE)
		, bShadowIndirectOnly(FALSE)
		, bUseEmissiveForStaticLighting(FALSE)
	    , EmissiveLightFalloffExponent(2.0f)
		, EmissiveLightExplicitInfluenceRadius(0.0f)
		, EmissiveBoost(1.0f)
		, DiffuseBoost(1.0f)
		, SpecularBoost(1.0f)
		, FullyOccludedSamplesFraction(1.0f)
	{
	}
};

enum EMeshInstanceLightingFlags
{
	/** Whether the mesh casts a shadow. */
	GI_INSTANCE_CASTSHADOW			= 1<<0,
	/** Whether the mesh uses a two-sided material. */
	GI_INSTANCE_TWOSIDED			= 1<<1,
	/** Whether the mesh only casts a shadow on itself. */
	GI_INSTANCE_SELFSHADOWONLY		= 1<<2,
	/** Whether to disable casting a shadow on itself. */
	GI_INSTANCE_SELFSHADOWDISABLE	= 1<<3
};

struct FStaticLightingMeshInstanceData
{
	FGuid Guid;
	/** The number of triangles in the mesh, used for visibility testing. */
	INT NumTriangles;
	/** The number of shading triangles in the mesh. */
	INT NumShadingTriangles;
	/** The number of vertices in the mesh, used for visibility testing. */
	INT NumVertices;
	/** The number of shading vertices in the mesh. */
	INT NumShadingVertices;
	/** The texture coordinate index which is used to parametrize materials. */
	INT TextureCoordinateIndex;
	INT MeshIndex;
	INT LevelId;
	DWORD LightingFlags;		// EMeshInstanceLightingFlags
	UBOOL bCastShadowAsTwoSided;
	/** Whether the mesh can be moved in game or not. */
	UBOOL bMovable;
	/** Whether this is an instanced static mesh */
	UBOOL bInstancedStaticMesh;
	/** The lights which affect the mesh's primitive. */
	INT NumRelevantLights;
	/** The bounding box of the mesh. */
	FBox BoundingBox;
};

/** 
 * Parameters used to transform a static mesh based on a spline.
 * Be sure to update this structure if spline functionality changes in UE3!
 */
struct FSplineMeshParams
{
	/** Start location of spline, in component space */
	FVector4 StartPos;
	/** Start tangent of spline, in component space */
	FVector4 StartTangent;
	/** X and Y scale applied to mesh at start of spline */
	FVector2D StartScale;
	/** Roll around spline applied at start */
	FLOAT StartRoll;
	/** Offset from the spline at start */
	FVector2D StartOffset;
	/** End location of spline, in component space */
	FVector4 EndPos;
	/** End tangent of spline, in component space */
	FVector4 EndTangent;
	/** X and Y scale applied to mesh at end of spline */
	FVector2D EndScale;
	/** Roll around spline applied at end */
	FLOAT EndRoll;
	/** Offset from the base spline at end */
	FVector2D EndOffset;
	/** Axis (in component space) that is used to determine X axis for co-ordinates along spline */
	FVector4 SplineXDir;
	/** Smoothly (cubic) interpolate the Roll and Scale params over spline. */
	UBOOL bSmoothInterpRollScale;
	/** Minimum Z value of the entire mesh */
	FLOAT MeshMinZ;
	/** Range of Z values over entire mesh */
	FLOAT MeshRangeZ;
};

struct FStaticMeshStaticLightingMeshData
{
	/** The LOD this mesh represents. */
	INT EncodedLODIndex;
	FMatrix LocalToWorld;
	/** TRUE if the primitive has a transform which reverses the winding of its triangles. */
	UBOOL bReverseWinding;
	UBOOL bShouldSelfShadow;
	FGuid StaticMeshGuid;
	UBOOL bIsSplineMesh;
	FSplineMeshParams SplineParameters;
};

struct FStaticLightingVertexData
{
	FVector4 WorldPosition;
	FVector4 WorldTangentX;
	FVector4 WorldTangentY;
	FVector4 WorldTangentZ;
	FVector2D TextureCoordinates[MAX_TEXCOORDS];
};

struct FBSPSurfaceStaticLightingData
{
	FVector4	TangentX;
	FVector4	TangentY;
	FVector4	TangentZ;

	FMatrix		MapToWorld;
	FMatrix		WorldToMap;

	FGuid		MaterialGuid;
};

struct FStaticLightingMappingData
{
	FGuid Guid;
	TCHAR UniqueName[256];
	FGuid StaticLightingMeshInstance;
	/** TRUE if light-maps to be used for the object's direct lighting. */
	UBOOL bForceDirectLightMap;
};

struct FStaticLightingTextureMappingData
{
	/** The width of the static lighting textures used by the mapping. */
	INT SizeX;
	/** The height of the static lighting textures used by the mapping. */
	INT SizeY;
	/** The lightmap texture coordinate index which is used for the mapping. */
	INT LightmapTextureCoordinateIndex;
	/** Whether to apply a bilinear filter to the sample or not. */
	UBOOL bBilinearFilter;
};

struct FStaticLightingVertexMappingData
{
	/** Lighting will be sampled at a random number of samples/surface area proportional to this factor. */
	FLOAT SampleToAreaRatio;
	/** TRUE to sample at vertices instead of on the surfaces. */
	UBOOL bSampleVertices;
};

//
//	Terrain
//
struct FTerrainPatchBoundsData
{
	FLOAT	MinHeight;
 	FLOAT	MaxHeight;
 	FLOAT	MaxDisplacement;
};

struct FStaticLightingTerrainMappingData
{
	/** The terrain this object represents. */
	//ATerrain* const Terrain;
	FGuid TerrainGuid;

	/** The primitive this object represents. */
	INT SectionBaseX;
	INT SectionBaseY;
	INT SectionSizeX;
	INT SectionSizeY;
	/** The actual section size in vertices...										*/
	INT TrueSectionSizeX;
	INT TrueSectionSizeY;
	/** The number of quads in the component along the X axis. */
	INT NumQuadsX;
	/** The number of quads in the component along the Y axis. */
	INT NumQuadsY;
	/** The number of quads we are expanding to eliminate seams. */
	INT ExpandQuadsX;
	INT ExpandQuadsY;
	/** The inverse transpose of the primitive's local to world transform. */
	FMatrix LocalToWorldInverseTranspose;
};

//
//	Fluid surfaces
//
struct FFluidSurfaceStaticLightingMeshData
{
	/** The primitive's local to world transform. */
	FMatrix LocalToWorld;
	/** The inverse transpose of the primitive's local to world transform. */
	FMatrix LocalToWorldInverseTranspose;
	/** The mesh data of the fluid surface, which is represented as a quad. */
	FVector4 QuadCorners[4];
	FVector4 QuadUVCorners[4];
	INT QuadIndices[6];
};

//
// Landscape
//
struct FLandscapeStaticLightingMeshData
{
	/** The primitive's local to world transform. */
	FMatrix LocalToWorld;
	/** The inverse transpose of the primitive's local to world transform. */
	FMatrix LocalToWorldInverseTranspose;

	//FGuid LandscapeGuid; // for height map?
	INT SectionBaseX;
	INT SectionBaseY;

	// offset of this component's data into heightmap texture
	INT HeightmapStride;
	INT HeightmapComponentOffsetX;
	INT HeightmapComponentOffsetY;
	INT HeightmapSubsectionOffset;

	INT ComponentSizeQuads;
	INT SubsectionSizeQuads;
	INT SubsectionSizeVerts;

	FLOAT StaticLightingResolution;
	FLOAT LightMapRatio;
	INT SizeX;
	INT SizeY;
	/** The number of quads we are expanding to eliminate seams. */
	INT ExpandQuadsX;
	INT ExpandQuadsY;

	//	INT WeightmapStride;
	//	INT WeightmapOffsetX;
	//	INT WeightmapOffsetY;
	//	INT ComponentNumSubsections;
};

//
//	Speed Tree mesh type, mirrored from UE3
//

enum ESpeedTreeMeshType
{
	STMT_MinMinusOne,
	STMT_Branches1,
	STMT_Branches2,
	STMT_Fronds,
	STMT_LeafCards,
	STMT_LeafMeshes,
	STMT_Billboards,
	STMT_Max
};

/**
 * Data for a mapping instance for a speed tree primitive
 */
struct FSpeedTreeStaticLightingMeshData
{
	/** Guid of the ST instance for this mapping */
	FGuid InstanceSpeedTreeGuid;

	/** The LOD this mesh is part of. */
	UINT LODIndex;
	
	/** Type of mesh this mapping is (one of ESpeedTreeMeshType) */
	ESpeedTreeMeshType MeshType;

	/** Mesh element for this sub-mesh */
	UINT ElementFirstIndex;
	UINT ElementMinVertexIndex;

	/** ID of the component this came from (for tying lighting meshes together) */
	FGuid ComponentGuid;

	/** The primitive's local to world transform. */
	FMatrix LocalToWorld;
};

#pragma pack(pop)

}	// namespace Lightmass

