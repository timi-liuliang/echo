/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_DESTRUCTIBLE_ASSET_H
#define NX_DESTRUCTIBLE_ASSET_H

#define NX_DESTRUCTIBLE_AUTHORING_TYPE_NAME "DestructibleAsset"

#include "foundation/Px.h"
#include "NxFractureTools.h"
// TODO: Remove this include when we remove the APEX_RUNTIME_FRACTURE define
#include "NxModuleDestructible.h"

#if NX_SDK_VERSION_MAJOR == 3
#include "PxFiltering.h"
#endif

namespace physx
{
namespace apex
{

class NxDestructibleActor;
class NxDestructiblePreview;

PX_PUSH_PACK_DEFAULT
#if !defined(PX_PS4)
	#pragma warning(push)
	#pragma warning(disable:4121)
#endif	//!PX_PS4

/**
	Flags that may be set for all chunks at a particular depth in the fracture hierarchy
*/
struct NxDestructibleDepthParametersFlag
{
	enum Enum
	{
		/*
			If true, chunks at this hierarchy depth level will take impact damage iff OVERRIDE_IMPACT_DAMAGE = TRUE, no matter the setting of impactDamageDefaultDepth..
			Note, NxDestructibleParameters::forceToDamage must also be positive for this
			to take effect.
		*/
		OVERRIDE_IMPACT_DAMAGE			= (1 << 0),

		/**
			If OVERRIDE_IMPACT_DAMAGE = TRUE, chunks at this hierarchy depth level will take impact damage iff OVERRIDE_IMPACT_DAMAGE = TRUE, no matter the setting of impactDamageDefaultDepth..
		*/
		OVERRIDE_IMPACT_DAMAGE_VALUE	= (1 << 1),

		/**
			Chunks at this depth should have pose updates ignored.
		*/
		IGNORE_POSE_UPDATES				= (1 << 2),

		/**
			Chunks at this depth should be ignored in raycast callbacks.
		*/
		IGNORE_RAYCAST_CALLBACKS		= (1 << 3),

		/**
			Chunks at this depth should be ignored in contact callbacks.
		*/
		IGNORE_CONTACT_CALLBACKS		= (1 << 4),

		/**
			User defined flags.
		*/
		USER_FLAG_0						= (1 << 24),
		USER_FLAG_1						= (1 << 25),
		USER_FLAG_2						= (1 << 26),
		USER_FLAG_3						= (1 << 27)
	};
};


/**
	Parameters that may be set for all chunks at a particular depth in the fracture hierarchy
*/
struct NxDestructibleDepthParameters
{
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleDepthParameters();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
	\brief (re)sets the structure to parameters which are likely to be the most computationally expensive.
	*/
	PX_INLINE void	setToMostExpensive();

	/**
		A convenience function to determine if the OVERRIDE_IMPACT_DAMAGE is set.
	*/
	PX_INLINE bool	overrideImpactDamage() const;

	/**
		A convenience function to read OVERRIDE_IMPACT_DAMAGE_VALUE is set.
	*/
	PX_INLINE bool	overrideImpactDamageValue() const;

	/**
		A convenience function to determine if the IGNORE_POSE_UPDATES is set.
	*/
	PX_INLINE bool	ignoresPoseUpdates() const;

	/**
		A convenience function to determine if the IGNORE_RAYCAST_CALLBACKS is set.
	*/
	PX_INLINE bool	ignoresRaycastCallbacks() const;

	/**
		A convenience function to determine if the IGNORE_CONTACT_CALLBACKS is set.
	*/
	PX_INLINE bool	ignoresContactCallbacks() const;

	/**
		A convenience function to determine if the USER_FLAG_0, USER_FLAG_1, USER_FLAG_2, or USER_FLAG_3 flag is set.
	*/
	PX_INLINE bool	hasUserFlagSet(physx::PxU32 flagIndex) const;

	/**
		A collection of flags defined in NxDestructibleDepthParametersFlag.
	*/
	physx::PxU32	flags;
};

// NxDestructibleDepthParameters inline functions

PX_INLINE NxDestructibleDepthParameters::NxDestructibleDepthParameters()
{
	setToDefault();
}

PX_INLINE void NxDestructibleDepthParameters::setToDefault()
{
	flags = 0;
}

PX_INLINE bool NxDestructibleDepthParameters::overrideImpactDamage() const
{
	return (flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE) != 0;
}

PX_INLINE bool NxDestructibleDepthParameters::overrideImpactDamageValue() const
{
	return (flags & NxDestructibleDepthParametersFlag::OVERRIDE_IMPACT_DAMAGE_VALUE) != 0;
}

PX_INLINE bool NxDestructibleDepthParameters::ignoresPoseUpdates() const
{
	return (flags & NxDestructibleDepthParametersFlag::IGNORE_POSE_UPDATES) != 0;
}

PX_INLINE bool NxDestructibleDepthParameters::ignoresRaycastCallbacks() const
{
	return (flags & NxDestructibleDepthParametersFlag::IGNORE_RAYCAST_CALLBACKS) != 0;
}

PX_INLINE bool NxDestructibleDepthParameters::ignoresContactCallbacks() const
{
	return (flags & NxDestructibleDepthParametersFlag::IGNORE_CONTACT_CALLBACKS) != 0;
}

PX_INLINE bool NxDestructibleDepthParameters::hasUserFlagSet(physx::PxU32 flagIndex) const
{
	switch (flagIndex)
	{
	case 0:
		return (flags & NxDestructibleDepthParametersFlag::USER_FLAG_0) != 0;
	case 1:
		return (flags & NxDestructibleDepthParametersFlag::USER_FLAG_1) != 0;
	case 2:
		return (flags & NxDestructibleDepthParametersFlag::USER_FLAG_2) != 0;
	case 3:
		return (flags & NxDestructibleDepthParametersFlag::USER_FLAG_3) != 0;
	default:
		return false;
	}
}

/**
	Parameters for RT Fracture
*/
struct NxDestructibleRTFractureParameters
{
	/**
		If true, align fracture pattern to largest face. 
		If false, the fracture pattern will be aligned to the hit normal with each fracture.
	*/
	bool sheetFracture;

	/**
		Number of times deep a chunk can be fractured. Can help limit the number of chunks produced by
		runtime fracture.
	*/
	physx::PxU32 depthLimit;

	/**
		If true, destroy chunks when they hit their depth limit. 
		If false, then chunks at their depth limit will not fracture but will have a force applied.
	*/
	bool destroyIfAtDepthLimit;

	/**
		Minimum Convex Size. Minimum size of convex produced by a fracture.
	*/
	physx::PxF32 minConvexSize;

	/**
		Scales impulse applied by a fracture.
	*/
	physx::PxF32 impulseScale;

	struct FractureGlass 
	{
		/**
			Number of angular slices in the glass fracture pattern.
		*/
		physx::PxU32 numSectors;

		/**
			Creates variance in the angle of slices. A value of zero results in all angular slices having the same angle.
		*/
		physx::PxF32 sectorRand;

		/**
			The minimum shard size. Shards below this size will not be created and thus not visible.
		*/
		physx::PxF32 firstSegmentSize;

		/**
			Scales the radial spacing in the glass fracture pattern. A larger value results in radially longer shards.
		*/
		physx::PxF32 segmentScale;

		/**
			Creates variance in the radial size of shards. A value of zero results in a low noise circular pattern.
		*/
		physx::PxF32 segmentRand;
	}glass;

	struct FractureAttachment 
	{
		/**
			If true, make the positive x side of the sheet an attachment point.
		*/
		bool posX;

		/**
			If true, make the negative x side of the sheet an attachment point.
		*/
		bool negX;

		/**
			If true, make the positive y side of the sheet an attachment point.
		*/
		bool posY;

		/**
			If true, make the negative y side of the sheet an attachment point.
		*/
		bool negY;

		/**
			If true, make the positive z side of the sheet an attachment point.
		*/
		bool posZ;

		/**
			If true, make the negative z side of the sheet an attachment point.
		*/
		bool negZ;			
	}attachment;

	PX_INLINE void	setToDefault();
};

PX_INLINE void NxDestructibleRTFractureParameters::setToDefault()
{
	sheetFracture = true;
	depthLimit = 2;
	destroyIfAtDepthLimit = false;
	minConvexSize = 0.02f;
	impulseScale = 1.0f;
	glass.numSectors = 10;
	glass.sectorRand = 0.3f;
	glass.firstSegmentSize = 0.06f;
	glass.segmentScale = 1.4f;
	glass.segmentRand = 0.3f;
	attachment.posX = false;
	attachment.negX = false;
	attachment.posY = false;
	attachment.negY = false;
	attachment.posZ = false;
	attachment.negZ = false;
}

/**
	Flags that apply to a destructible actor, settable at runtime
*/
struct NxDestructibleParametersFlag
{
	enum Enum
	{
		/**
			If set, chunks will "remember" damage applied to them, so that many applications of a damage amount
			below damageThreshold will eventually fracture the chunk.  If not set, a single application of
			damage must exceed damageThreshold in order to fracture the chunk.
		*/
		ACCUMULATE_DAMAGE =	(1 << 0),

		/**
			Whether or not chunks at or deeper than the "debris" depth (see NxDestructibleParameters::debrisDepth)
			will time out.  The lifetime is a value between NxDestructibleParameters::debrisLifetimeMin and
			NxDestructibleParameters::debrisLifetimeMax, based upon the destructible module's LOD setting.
		*/
		DEBRIS_TIMEOUT =	(1 << 1),

		/**
			Whether or not chunks at or deeper than the "debris" depth (see NxDestructibleParameters::debrisDepth)
			will be removed if they separate too far from their origins.  The maxSeparation is a value between
			NxDestructibleParameters::debrisMaxSeparationMin and NxDestructibleParameters::debrisMaxSeparationMax,
			based upon the destructible module's LOD setting.
		*/
		DEBRIS_MAX_SEPARATION =	(1 << 2),

		/**
			If set, the smallest chunks may be further broken down, either by fluid crumbles (if a crumble particle
			system is specified in the NxDestructibleActorDesc), or by simply removing the chunk if no crumble
			particle system is specified.  Note: the "smallest chunks" are normally defined to be the deepest level
			of the fracture hierarchy.  However, they may be taken from higher levels of the hierarchy if
			NxModuleDestructible::setMaxChunkDepthOffset is called with a non-zero value.
		*/
		CRUMBLE_SMALLEST_CHUNKS =	(1 << 3),

		/**
			If set, the NxDestructibleActor::rayCast function will search within the nearest visible chunk hit
			for collisions with child chunks.  This is used to get a better raycast position and normal, in
			case the parent collision volume does not tightly fit the graphics mesh.  The returned chunk index
			will always be that of the visible parent that is intersected, however.
		*/
		ACCURATE_RAYCASTS =	(1 << 4),

		/**
			If set, the validBounds field of NxDestructibleParameters will be used.  These bounds are translated
			(but not scaled or rotated) to the origin of the destructible actor.  If a chunk or chunk island moves
			outside of those bounds, it is destroyed.
		*/
		USE_VALID_BOUNDS =	(1 << 5),

		/**
			If set, chunk crumbling will be handled via the pattern-based runtime fracture pipeline.
			If no fracture pattern is specified in the NxDestructibleActorDesc, or no fracture pattern
			has been assigned to the destructible actor, chunks will simply be removed.
		*/
		CRUMBLE_VIA_RUNTIME_FRACTURE =	(1 << 6),
	};
};


/**
	Parameters that apply to a destructible actor
*/
struct NxDestructibleParameters
{
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleParameters();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		Limits the amount of damage applied to a chunk.  This is useful for preventing the entire destructible
		from getting pulverized by a very large application of damage.  This can easily happen when impact damage is
		used, and the damage amount is proportional to the impact force (see forceToDamage).
	*/
	physx::PxF32		damageCap;

	/**
		If a chunk is at a depth which takes impact damage (see NxDestructibleDepthParameters),
		then when a chunk has a collision in the NxScene, it will take damage equal to forceToDamage mulitplied by
		the impact force.
		The default value is zero, which effectively disables impact damage.
	*/
	physx::PxF32		forceToDamage;

	/**
		Large impact force may be reported if rigid bodies are spawned inside one another.  In this case the relative velocity of the two
		objects will be low.  This variable allows the user to set a minimum velocity threshold for impacts to ensure that the objects are
		moving at a min velocity in order for the impact force to be considered.
		Default value is zero.
	*/
	physx::PxF32		impactVelocityThreshold;

	/**
		The chunks will not be broken free below this depth.
	*/
	physx::PxU32		minimumFractureDepth;

	/**
		The default depth to which chunks will take impact damage.  This default may be overridden in the depth settings.
		Negative values imply no default impact damage.
		Default value = -1.
	*/
	physx::PxI32		impactDamageDefaultDepth;

	/**
		The chunk hierarchy depth at which chunks are considered to be "debris."  Chunks at this depth or
		below will be considered for various debris settings, such as debrisLifetime.
		Negative values indicate that no chunk depth is considered debris.
		Default value is -1.
	*/
	physx::PxI32		debrisDepth;

	/**
		The chunk hierarchy depth up to which chunks will not be eliminated due to LOD considerations.
		These chunks are considered to be essential either for gameplay or visually.
		The minimum value is 0, meaning the level 0 chunk is always considered essential.
		Default value is 0.
	*/
	physx::PxU32		essentialDepth;

	/**
		"Debris chunks" (see debrisDepth, above) will be destroyed after a time (in seconds)
		separated from non-debris chunks.  The actual lifetime is interpolated between these
		two values, based upon the module's LOD setting.  To disable lifetime, clear the
		NxDestructibleDepthParametersFlag::DEBRIS_TIMEOUT flag in the flags field.
		If debrisLifetimeMax < debrisLifetimeMin, the mean of the two is used for both.
		Default debrisLifetimeMin = 1.0, debrisLifetimeMax = 10.0f.
	*/
	physx::PxF32		debrisLifetimeMin;
	physx::PxF32		debrisLifetimeMax;

	/**
		"Debris chunks" (see debrisDepth, above) will be destroyed if they are separated from
		their origin by a distance greater than maxSeparation.  The actual maxSeparation is
		interpolated between these two values, based upon the module's LOD setting.  To disable
		maxSeparation, clear the NxDestructibleDepthParametersFlag::DEBRIS_MAX_SEPARATION flag in the flags field.
		If debrisMaxSeparationMax < debrisMaxSeparationMin, the mean of the two is used for both.
		Default debrisMaxSeparationMin = 1.0, debrisMaxSeparationMax = 10.0f.
	*/
	physx::PxF32		debrisMaxSeparationMin;
	physx::PxF32		debrisMaxSeparationMax;

	/**
		The probablity that a debris chunk, when fractured, will simply be destroyed instead of becoming
		dynamic or breaking down further into child chunks.  Valid range = [0.0,1.0].  Default value = 0.0.'
	*/

	physx::PxF32		debrisDestructionProbability;

	/**
		A bounding box around each NxDestructibleActor created, defining a range of validity for chunks that break free.
		These bounds are scaled and translated with the NxDestructibleActor's scale and position, but they are *not*
		rotated with the NxDestructibleActor.
	*/
	physx::PxBounds3	validBounds;

	/**
		If greater than 0, the chunks' speeds will not be allowed to exceed maxChunkSpeed.  Use 0
		to disable this feature (this is the default).
	*/
	physx::PxF32		maxChunkSpeed;

	/**
		A collection of flags defined in NxDestructibleParametersFlag.
	*/
	physx::PxU32		flags;

	/**
		Scale factor used to apply an impulse force along the normal of chunk when fractured.  This is used
		in order to "push" the pieces out as they fracture.
	*/
	physx::PxF32		fractureImpulseScale;

	/**
		How deep in the hierarchy damage will be propagated, relative to the chunk hit.
		Default = PX_MAX_U16.
	*/
	physx::PxU16		damageDepthLimit;

	/**
		Optional dominance group for dynamic chunks created when fractured. (ignored if > 31)
	*/
	physx::PxU8			dynamicChunksDominanceGroup;

	/**
		Whether or not to use dynamicChunksGroupsMask.  If false, NULL will be passed into the DestructibleActor upon
		instantiation, through the NxDestructibleActorDesc.
	*/
	bool				useDynamicChunksGroupsMask;

	/**
		Optional groups mask (2.x) or filter data (3.x) for dynamic chunks created when fractured. (Used if useDynamicChunksGroupsMask is true.)
	*/
#if (NX_SDK_VERSION_MAJOR == 2)
	NxGroupsMask		dynamicChunksGroupsMask;
#elif (NX_SDK_VERSION_MAJOR == 3)
	physx::PxFilterData	dynamicChunksFilterData;
	/**
		The supportStrength is used for the stress solver.  As this value is increased it takes more force to break apart the chunks under the effect of stress solver.
	*/
	physx::PxF32	   supportStrength;
#endif

	/**
		Whether or not to use the old chunk bounds testing for damage, or use the module setting.  A value of 0 forces the new method to be used.
		A positive value forces the old method to be used.  Negative values cause the global (NxModuleDestructible) setting to be used.
		Default = -1
	*/
	physx::PxI8			legacyChunkBoundsTestSetting;

	/**
		Whether or not to use the old damage spread method, or use the module setting.  A value of 0 forces the new method to be used.
		A positive value forces the old method to be used.  Negative values cause the global (NxModuleDestructible) setting to be used.
		Default = -1
	*/
	physx::PxI8			legacyDamageRadiusSpreadSetting;

	/**
		The maximum number of NxDestructibleDepthParameters (see depthParameters).
	*/
	enum { kDepthParametersCountMax = 16 };

	/**
		The number of NxDestructibleDepthParameters (see depthParameters).
		Must be in the range [0, kDepthParametersCountMax].
	*/
	physx::PxU32		depthParametersCount;

	/**
		Parameters that apply to every chunk at a given depth level (see NxDestructibleDepthParameters).
		The element [0] of the array applies to the depth 0 (unfractured) chunk, element [1] applies
		to the level 1 chunks, etc.
		The number of valid depth parameters must be given in depthParametersCount.
	*/
	NxDestructibleDepthParameters	depthParameters[kDepthParametersCountMax];

	/**
		Parameters for RT Fracture.
	*/
	NxDestructibleRTFractureParameters rtFractureParameters;
};

// NxDestructibleParameters inline functions

PX_INLINE NxDestructibleParameters::NxDestructibleParameters()
{
	setToDefault();
}

PX_INLINE void NxDestructibleParameters::setToDefault()
{
	damageCap = 0;
	forceToDamage = 0;
	impactVelocityThreshold = 0.0f;
	minimumFractureDepth = 0;
	impactDamageDefaultDepth = -1;
	debrisDepth = -1;
	essentialDepth = 0;
	debrisLifetimeMin = 1.0f;
	debrisLifetimeMax = 10.0f;
	debrisMaxSeparationMin = 1.0f;
	debrisMaxSeparationMax = 10.0f;
	debrisDestructionProbability = 0.0f;
	validBounds = physx::PxBounds3(physx::PxVec3(-10000.0f), physx::PxVec3(10000.0f));
	maxChunkSpeed = 0.0f;
	fractureImpulseScale = 0.0f;
	damageDepthLimit = PX_MAX_U16;
	useDynamicChunksGroupsMask = false;
#if (NX_SDK_VERSION_MAJOR == 2)
	dynamicChunksGroupsMask.bits0 = dynamicChunksGroupsMask.bits1 = dynamicChunksGroupsMask.bits2 = dynamicChunksGroupsMask.bits3 = 0;
#elif  (NX_SDK_VERSION_MAJOR == 3)
	dynamicChunksFilterData.word0 = dynamicChunksFilterData.word1 = dynamicChunksFilterData.word2 = dynamicChunksFilterData.word3 = 0;
	supportStrength = -1.0;
#endif
	legacyChunkBoundsTestSetting = -1;
	legacyDamageRadiusSpreadSetting = -1;
	dynamicChunksDominanceGroup = 0xFF;	// Out of range, so it won't be used.
	flags = NxDestructibleParametersFlag::ACCUMULATE_DAMAGE;
	depthParametersCount = 0;
	rtFractureParameters.setToDefault();
}


struct NxDestructibleInitParametersFlag
{
	enum Enum
	{
		/**
			If set, then chunks which are tagged as "support" chunks (via NxDestructibleChunkDesc::isSupportChunk)
			will have environmental support in static destructibles.
			Note: if both ASSET_DEFINED_SUPPORT and WORLD_SUPPORT are set, then chunks must be tagged as
			"support" chunks AND overlap the NxScene's static geometry in order to be environmentally supported.
		*/
		ASSET_DEFINED_SUPPORT =	(1 << 0),

		/**
			If set, then chunks which overlap the NxScene's static geometry will have environmental support in
			static destructibles.
			Note: if both ASSET_DEFINED_SUPPORT and WORLD_SUPPORT are set, then chunks must be tagged as
			"support" chunks AND overlap the NxScene's static geometry in order to be environmentally supported.
		*/
		WORLD_SUPPORT =	(1 << 1),

		/**
			If this is set and the destructible is initially static, it will become part of an extended support
			structure if it is in contact with another static destructible that also has this flag set.
		*/
		FORM_EXTENDED_STRUCTURES =	(1 << 2)
	};
};

/**
	Parameters that apply to a destructible actor at initialization
*/
struct NxDestructibleInitParameters
{
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleInitParameters();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		The chunk hierarchy depth at which to create a support graph.  Higher depth levels give more detailed support,
		but will give a higher computational load.  Chunks below the support depth will never be supported.
	*/
	physx::PxU32		supportDepth;

	/**
		A collection of flags defined in NxDestructibleInitParametersFlag.
	*/
	physx::PxU32		flags;
};


// NxDestructibleInitParameters inline functions

PX_INLINE NxDestructibleInitParameters::NxDestructibleInitParameters()
{
	setToDefault();
}

PX_INLINE void NxDestructibleInitParameters::setToDefault()
{
	supportDepth = 0;
	flags = 0;
}

// NxDamageSpreadFunction
struct NxDamageSpreadFunction
{
	PX_INLINE		NxDamageSpreadFunction()
	{
		setToDefault();
	}

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault()
	{
		minimumRadius = 0.0f;
		radiusMultiplier = 1.0f;
		falloffExponent = 1.0f;
	}

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const
	{
		return
			minimumRadius >= 0.0f &&
			radiusMultiplier >= 0.0f &&
			falloffExponent >= 0.0f;
	}

	physx::PxF32 minimumRadius;
	physx::PxF32 radiusMultiplier;
	physx::PxF32 falloffExponent;
};

/**
	Destructible authoring structure.

	Descriptor to build one chunk in a fracture hierarchy.
*/
class NxDestructibleBehaviorGroupDesc : public NxApexDesc
{
public:
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleBehaviorGroupDesc();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const;

	/**
		Defines the chunk to be environmentally supported, if the appropriate NxDestructibleParametersFlag flags
		are set in NxDestructibleParameters.
	*/
	const char*				name;
	physx::PxF32			damageThreshold;
	physx::PxF32			damageToRadius;
	NxDamageSpreadFunction	damageSpread;
	NxDamageSpreadFunction	damageColorSpread;
	physx::PxVec4			damageColorChange;
	physx::PxF32			materialStrength;
	physx::PxF32			density;
	physx::PxF32			fadeOut;
	physx::PxF32			maxDepenetrationVelocity;
	physx::PxU64			userData;
};

// NxDestructibleChunkDesc inline functions

PX_INLINE NxDestructibleBehaviorGroupDesc::NxDestructibleBehaviorGroupDesc()
{
	setToDefault();
}

PX_INLINE void NxDestructibleBehaviorGroupDesc::setToDefault()
{
	NxApexDesc::setToDefault();

	// TC_TODO: suitable default values?
	name = NULL;
	damageThreshold = 1.0f;
	damageToRadius = 0.1f;
	damageSpread.setToDefault();
	damageColorSpread.setToDefault();
	damageColorChange.setZero();
	materialStrength = 0.0f;
	density = 0;
	fadeOut = 1;
	maxDepenetrationVelocity = PX_MAX_F32;
	userData = (physx::PxU64)0;
}

PX_INLINE bool NxDestructibleBehaviorGroupDesc::isValid() const
{
	// TC_TODO: this good enough?
	if (damageThreshold < 0 ||
		damageToRadius < 0 ||
		!damageSpread.isValid() ||
		!damageColorSpread.isValid() ||
		materialStrength < 0 ||
		density < 0 ||
		fadeOut < 0 ||
		!(maxDepenetrationVelocity > 0.f))
	{
		return false;
	}

	return NxApexDesc::isValid();
}
/**
	Destructible authoring structure.

	Descriptor to build one chunk in a fracture hierarchy.
*/
class NxDestructibleChunkDesc : public NxApexDesc
{
public:
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleChunkDesc();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const;

	/**
		Defines the chunk to be environmentally supported, if the appropriate NxDestructibleParametersFlag flags
		are set in NxDestructibleParameters.
	*/
	bool					isSupportChunk;

	/**
		Defines the chunk to be unfractureable.  If this is true, then none of its children will be fractureable.
	*/
	bool					doNotFracture;

	/**
		Defines the chunk to be undamageable.  This means this chunk will not fracture, but its children might.
	*/
	bool					doNotDamage;

	/**
		Defines the chunk to be uncrumbleable.  This means this chunk will not be broken down into fluid mesh particles
		no matter how much damage it takes.  Note: this only applies to chunks with no children.  For a chunk with
		children, then:
		1) The chunk may be broken down into its children, and then its children may be crumbled, if the doNotCrumble flag
		is not set on them.
		2) If the Destructible module's chunk depth offset LOD may be set such that this chunk effectively has no children.
		In this case, the doNotCrumble flag will apply to it.
	*/
	bool					doNotCrumble;

#if APEX_RUNTIME_FRACTURE
	/**
		Defines the chunk to use run-time, dynamic fracturing.  The chunk will use the fracture pattern provided by the asset
		to guide it's fracture.  The resulting chunks will follow a similar process for subdivision.
	*/
	bool					runtimeFracture;
#endif

	/**
		Whether or not to use instancing when rendering this chunk.  If useInstancedRendering = TRUE, this chunk will
		share a draw call with all others that instance the mesh indexed by meshIndex.  This may extend to other
		destructible actors created from this asset.  If useInstancedRendering = FALSE, this chunk may share a draw
		call only with other chunks in this asset which have useInstancedRendering = FALSE.
		Default = FALSE.
	*/
	bool					useInstancedRendering;

	/**
		Translation for this chunk mesh within the asset. Normally a chunk needs no translation, but if a chunk is instanced within
		the asset, then this translation is needed.
		Default = (0,0,0).
	*/
	physx::PxVec3			instancePositionOffset;

	/**
		UV translation for this chunk mesh within the asset. Normally a chunk needs no UV translation, but if a chunk is instanced within
		the asset, then this translation is usually needed.
		Default = (0,0).
	*/
	physx::PxVec2			instanceUVOffset;

	/**
		If useInstancedRendering = TRUE, this index is the instanced mesh index.  If useInstancedRendering = FALSE,
		this index is the mesh part index for the skinned or statically rendered mesh.
		This must index a valid NxDestructibleGeometryDesc (see below).
		Default = 0xFFFF (invalid).
	*/
	physx::PxU16			meshIndex;

	/**
		The parent index of this chunk.  If the index is negative, this is a root chunk.
		Default = -1.
	*/
	physx::PxI32			parentIndex;

	/**
		A direction used to move the chunk out of the destructible, if an impact kick is applied.
	*/
	physx::PxVec3			surfaceNormal;

	/**
		Referring to the behavior group used of this chunk, please check NxDestructibleAssetCookingDesc::behaviorGroupDescs for more detail.
	*/
	physx::PxI8				behaviorGroupIndex;

	/**
		The number of scatter mesh instances on this chunk.
		Default = 0.
	*/
	physx::PxU32			scatterMeshCount;

	/**
		Array of indices corresponding the scatter mesh assets set using NxDestructibleAssetAuthoring::setScatterMeshAssets().
		The array length must be at least scatterMeshCount.  This pointer may be NULL if scatterMeshCount = 0.
		Default = NULL.
	*/
	const physx::PxU8*		scatterMeshIndices;

	/**
		Array of chunk-relative transforms corresponding the scatter mesh assets set using
		NxDestructibleAssetAuthoring::setScatterMeshAssets().
		The array length must be at least scatterMeshCount.  This pointer may be NULL if scatterMeshCount = 0.
		Default = NULL.
	*/
	const physx::PxMat44*	scatterMeshTransforms;
};

// NxDestructibleChunkDesc inline functions

PX_INLINE NxDestructibleChunkDesc::NxDestructibleChunkDesc()
{
	setToDefault();
}

PX_INLINE void NxDestructibleChunkDesc::setToDefault()
{
	NxApexDesc::setToDefault();
	isSupportChunk = false;
	doNotFracture = false;
	doNotDamage = false;
	doNotCrumble = false;
#if APEX_RUNTIME_FRACTURE
	runtimeFracture = false;
#endif
	useInstancedRendering = false;
	instancePositionOffset = physx::PxVec3(0.0f);
	instanceUVOffset = physx::PxVec2(0.0f);
	meshIndex = 0xFFFF;
	parentIndex = -1;
	surfaceNormal = physx::PxVec3(0.0f);
	behaviorGroupIndex = -1;
	scatterMeshCount = 0;
	scatterMeshIndices = NULL;
	scatterMeshTransforms = NULL;
}

PX_INLINE bool NxDestructibleChunkDesc::isValid() const
{
	if (meshIndex == 0xFFFF)
	{
		return false;
	}

	return NxApexDesc::isValid();
}


/**
	Destructible authoring structure.

	Descriptor to build one chunk in a fracture hierarchy.
*/
class NxDestructibleGeometryDesc : public NxApexDesc
{
public:
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleGeometryDesc();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const;

	/**
		The convex hulls associated with this chunk.  These may be obtained from NxExplicitHierarchicalMesh::convexHulls()
		if authoring using an NxExplicitHierarchicalMesh.  The length of the array is given by convexHullCount.
	*/
	const physx::NxExplicitHierarchicalMesh::NxConvexHull**	convexHulls;

	/**
		The length of the convexHulls array.  If this is positive, then convexHulls must point to a valid array of this size.
		If this is zero, then collisionVolumeDesc must not be NULL, and convex hulls will be automatically created for this
		geometry using collisionVolumeDesc.
	*/
	physx::PxU32											convexHullCount;

	/**
		If convexHullCount = 0, then collisionVolumeDesc must not be NULL.  In this case convex hulls will automatically be
		created for this geometry.  See NxCollisionVolumeDesc.
	*/
	const physx::NxCollisionVolumeDesc*						collisionVolumeDesc;
};

// NxDestructibleGeometryDesc inline functions

PX_INLINE NxDestructibleGeometryDesc::NxDestructibleGeometryDesc()
{
	setToDefault();
}

PX_INLINE void NxDestructibleGeometryDesc::setToDefault()
{
	NxApexDesc::setToDefault();
	convexHulls = NULL;
	convexHullCount = 0;
	collisionVolumeDesc = NULL;
}

PX_INLINE bool NxDestructibleGeometryDesc::isValid() const
{
	if (convexHullCount == 0 && collisionVolumeDesc == NULL)
	{
		return false;
	}

	if (convexHullCount > 0 && convexHulls == NULL)
	{
		return false;
	}

	return NxApexDesc::isValid();
}


/**
	Destructible authoring structure.

	Descriptor for the cookChunk() method of NxDestructibleAssetAuthoring
*/
class NxDestructibleAssetCookingDesc : public NxApexDesc
{
public:
	/**
	\brief constructor sets to default.
	*/
	PX_INLINE		NxDestructibleAssetCookingDesc();

	/**
	\brief (re)sets the structure to the default.
	*/
	PX_INLINE void	setToDefault();

	/**
		Returns true iff an object can be created using this descriptor.
	*/
	PX_INLINE bool	isValid() const;

	/**
		Beginning of array of descriptors, one for each chunk.
	*/
	NxDestructibleChunkDesc*			chunkDescs;

	/**
		The size of the chunkDescs array.  This must be positive.
	*/
	physx::PxU32						chunkDescCount;

	/**
		Default behavior group (corresponds to index of -1).
	*/
	NxDestructibleBehaviorGroupDesc		defaultBehaviorGroupDesc;

	/**
		Beginning of array of descriptors, one for each behavior group.
	*/
	NxDestructibleBehaviorGroupDesc*	behaviorGroupDescs;

	/**
		The size of the behaviorGroupDescs array.  This must be positive.
	*/
	physx::PxU32						behaviorGroupDescCount;

	physx::PxI8							RTFractureBehaviorGroup;

	/**
		Beginning of array of descriptors, one for each mesh part.
	*/
	NxDestructibleGeometryDesc*			geometryDescs;

	/**
		The size of the geometryDescs array.  This must be positive.
	*/
	physx::PxU32						geometryDescCount;

	/**
		Index pairs that represent chunk connections in the support graph.
		The indices refer to the chunkDescs array. Only sibling chunks,
		i.e. chunks at equal depth may be connected.
	*/
	physx::NxIntPair*					supportGraphEdges;

	/**
		Number of index pairs in supportGraphEdges.
	*/
	physx::PxU32						supportGraphEdgeCount;
};

// NxDestructibleAssetCookingDesc inline functions

PX_INLINE NxDestructibleAssetCookingDesc::NxDestructibleAssetCookingDesc()
{
	setToDefault();
}

PX_INLINE void NxDestructibleAssetCookingDesc::setToDefault()
{
	NxApexDesc::setToDefault();
	chunkDescs = NULL;
	chunkDescCount = 0;
	geometryDescs = NULL;
	geometryDescCount = 0;
	behaviorGroupDescs = 0;
	behaviorGroupDescCount = 0;
	supportGraphEdges = 0;
	supportGraphEdgeCount = 0;
}

PX_INLINE bool NxDestructibleAssetCookingDesc::isValid() const
{
	if (chunkDescCount == 0 || chunkDescs == NULL)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < chunkDescCount; ++i )
	{
		if (!chunkDescs[i].isValid())
		{
			return false;
		}
	}

	if (chunkDescCount >= 65535)
	{
		return false;
	}

	if (geometryDescCount == 0 || geometryDescs == NULL)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < geometryDescCount; ++i )
	{
		if (!geometryDescs[i].isValid())
		{
			return false;
		}
	}

	if (behaviorGroupDescCount > 127)
	{
		return false;
	}

	for (physx::PxU32 i = 0; i < behaviorGroupDescCount; ++i )
	{
		if (!behaviorGroupDescs[i].isValid())
		{
			return false;
		}
	}

	return NxApexDesc::isValid();
}


/**
	Stats for an NxDestructibleAsset: memory usage, counts, etc.
*/

struct NxDestructibleAssetStats
{
	physx::PxU32			totalBytes;
	physx::PxU32			chunkCount;
	physx::PxU32			chunkBytes;
	physx::PxU32			chunkHullDataBytes;
	physx::PxU32			collisionCookedHullDataBytes;
	physx::PxU32			collisionMeshCount;
	physx::PxU32			maxHullVertexCount;
	physx::PxU32			maxHullFaceCount;
	physx::PxU32			chunkWithMaxEdgeCount;
	physx::PxU32			runtimeCookedConvexCount;
	NxRenderMeshAssetStats	renderMeshAssetStats;
};

/**
	Authoring API for a destructible asset.
*/
class NxDestructibleAssetAuthoring : public NxApexAssetAuthoring
{
public:

	/** Fracturing API */

	/**
		NxDestructibleAssetAuthoring contains an instantiation of NxExplicitHierarchicalMesh.
		This function gives access to it.  See NxExplicitHierarchicalMesh for details, it is
		the object used by the fracturing tool set for mesh fracturing operations and is used
		to generate the embedded NxApexRenderMesh as well as collision and hierarchy data
		for the destructible asset.
	*/
	virtual NxExplicitHierarchicalMesh&		getExplicitHierarchicalMesh() = 0;

	/**
		NxDestructibleAssetAuthoring contains a second instantiation of NxExplicitHierarchicalMesh
		used to describe the core mesh for slice fracturing (see FractureTools::NxFractureSliceDesc),
		done in createHierarchicallySplitMesh().  This function gives access to it.
	*/
	virtual NxExplicitHierarchicalMesh&		getCoreExplicitHierarchicalMesh() = 0;

	/**
		NxDestructibleAssetAuthoring contains an instantiation of NxCutoutSet used to describe the
		cutout fracturing shapes (see FractureTools::NxCutoutSet), done in createChippedMesh().
		This function gives access to it.
	*/
	virtual FractureTools::NxCutoutSet&		getCutoutSet() = 0;

	/**
		Partitions (and possibly re-orders) the mesh array if the triangles form disjoint islands.

		\param mesh						pointer to array of NxExplicitRenderTriangles which make up the mesh
		\param meshTriangleCount		the size of the meshTriangles array
		\param meshPartition			user-allocated array for mesh partition, will be filled with the end elements of contiguous subsets of meshTriangles.
		\param meshPartitionMaxCount	size of user-allocated meshPartitionArray
		\param padding					relative value multiplied by the mesh bounding box. padding gets added to the triangle bounds when calculating triangle neighbors.

		\return							Returns the number of partitions.  The value may be larger than meshPartitionMaxCount.  In that case, the partitions beyond meshPartitionMaxCount are not recorded.
	*/
	virtual physx::PxU32					partitionMeshByIslands
	(
		physx::NxExplicitRenderTriangle* mesh,
		physx::PxU32 meshTriangleCount,
	    physx::PxU32* meshPartition,
	    physx::PxU32 meshPartitionMaxCount,
		physx::PxF32 padding = 0.0001f
	) = 0;

	/**
		Builds a new NxExplicitHierarchicalMesh from an array of triangles, used as the starting
		point for fracturing.  It will contain only one chunk, at depth 0.

		\param meshTriangles		pointer to array of NxExplicitRenderTriangles which make up the mesh
		\param meshTriangleCount	the size of the meshTriangles array
		\param submeshData			pointer to array of NxExplicitSubmeshData, describing the submeshes
		\param submeshCount			the size of the submeshData array
		\param meshPartition		if not NULL, an array of size meshPartitionCount, giving the end elements of contiguous subsets of meshTriangles.
										If meshPartition is NULL, one partition is assumed.
										When there is one partition, these triangles become the level 0 part.
										When there is more than one partition, the behavior is determined by firstPartitionIsDepthZero (see below).
		\param meshPartitionCount	if meshPartition is not NULL, this is the size of the meshPartition array.
		\param parentIndices		if not NULL, the parent indices for each chunk (corresponding to a partition in the mesh partition).
		\param parentIndexCount		the size of the parentIndices array.  This does not need to match meshPartitionCount.  If a mesh partition has an index beyond the end of parentIndices,
										then the parentIndex is considered to be 0.  Therefore, if parentIndexCount = 0, all parents are 0 and so all chunks created will be depth 1.  This will cause a
										depth 0 chunk to be created that is the aggregate of the depth 1 chunks.  If parentIndexCount > 0, then the depth-0 chunk must have a parentIndex of -1.
										To reproduce the effect of the old parameter 'firstPartitionIsDepthZero' = true, set parentIndices to the address of a PxI32 containing the value -1, and set parentIndexCount = 1.
										To reproduce the effect of the old parameter 'firstPartitionIsDepthZero' = false, set parentIndexCount = 0.
										Note: if parent indices are given, the first one must be -1, and *only* that index may be negative.  That is, there may be only one depth-0 mesh and it must be the first mesh.
	*/
	virtual bool							setRootMesh
	(
	    const NxExplicitRenderTriangle* meshTriangles,
	    physx::PxU32 meshTriangleCount,
	    const NxExplicitSubmeshData* submeshData,
	    physx::PxU32 submeshCount,
	    physx::PxU32* meshPartition = NULL,
	    physx::PxU32 meshPartitionCount = 0,
		physx::PxI32* parentIndices = NULL,
		physx::PxU32 parentIndexCount = 0
	) = 0;

	/** 
		Builds the root NxExplicitHierarchicalMesh from an NxRenderMeshAsset.
		Since an NxDestructibleAsset contains no hierarchy information, the input mesh must have only one part.

		\param renderMeshAsset		the asset to import
		\param maxRootDepth			cap the root depth at this value.  Re-fracturing of the mesh will occur at this depth.  Default = PX_MAX_U32
	*/
	virtual bool							importRenderMeshAssetToRootMesh(const physx::NxRenderMeshAsset& renderMeshAsset, physx::PxU32 maxRootDepth = PX_MAX_U32) = 0;

	/** 
		Builds the root NxExplicitHierarchicalMesh from an NxDestructibleAsset.
		Since an NxDestructibleAsset contains hierarchy information, the explicit mesh formed
		will have this hierarchy structure.

		\param destructibleAsset	the asset to import
		\param maxRootDepth			cap the root depth at this value.  Re-fracturing of the mesh will occur at this depth.  Default = PX_MAX_U32
	*/
	virtual bool							importDestructibleAssetToRootMesh(const physx::NxDestructibleAsset& destructibleAsset, physx::PxU32 maxRootDepth = PX_MAX_U32) = 0;

	/**
		Builds a new NxExplicitHierarchicalMesh from an array of triangles, used as the core mesh
		for slice fracture operations (see FractureTools::NxFractureSliceDesc, passed into
		createHierarchicallySplitMesh).

		\param mesh					pointer to array of NxExplicitRenderTriangles which make up the mesh
		\param meshTriangleCount	the size of the meshTriangles array
		\param submeshData			pointer to array of NxExplicitSubmeshData, describing the submeshes
		\param submeshCount			the size of the submeshData array
		\param meshPartition		meshPartition array
		\param meshPartitionCount	meshPartition array size
	*/
	virtual bool							setCoreMesh
	(
	    const NxExplicitRenderTriangle* mesh,
	    physx::PxU32 meshTriangleCount,
	    const NxExplicitSubmeshData* submeshData,
	    physx::PxU32 submeshCount,
	    physx::PxU32* meshPartition = NULL,
	    physx::PxU32 meshPartitionCount = 0
	) = 0;

	/**
		Builds a new NxExplicitHierarchicalMesh from an array of triangles, externally provided by the user.
		Note: setRootMesh and setCoreMesh may be implemented as follows:
			setRootMesh(x) <-> buildExplicitHierarchicalMesh( getExplicitHierarchicalMesh(), x)
			setCoreMesh(x) <-> buildExplicitHierarchicalMesh( getCoreExplicitHierarchicalMesh(), x)

		\param hMesh				new NxExplicitHierarchicalMesh
		\param meshTriangles		pointer to array of NxExplicitRenderTriangles which make up the mesh
		\param meshTriangleCount	the size of the meshTriangles array
		\param submeshData			pointer to array of NxExplicitSubmeshData, describing the submeshes
		\param submeshCount			the size of the submeshData array
		\param meshPartition		if not NULL, an array of size meshPartitionCount, giving the end elements of contiguous subsets of meshTriangles.
										If meshPartition is NULL, one partition is assumed.
										When there is one partition, these triangles become the level 0 part.
										When there is more than one partition, these triangles become level 1 parts, while the union of the parts will be the level 0 part.
		\param meshPartitionCount	if meshPartition is not NULL, this is the size of the meshPartition array.
		\param parentIndices		if not NULL, the parent indices for each chunk (corresponding to a partition in the mesh partition).
		\param parentIndexCount		the size of the parentIndices array.  This does not need to match meshPartitionCount.  If a mesh partition has an index beyond the end of parentIndices,
										then the parentIndex is considered to be 0.  Therefore, if parentIndexCount = 0, all parents are 0 and so all chunks created will be depth 1.  This will cause a
										depth 0 chunk to be created that is the aggregate of the depth 1 chunks.  If parentIndexCount > 0, then the depth-0 chunk must have a parentIndex of -1.
										To reproduce the effect of the old parameter 'firstPartitionIsDepthZero' = true, set parentIndices to the address of a PxI32 containing the value -1, and set parentIndexCount = 1.
										To reproduce the effect of the old parameter 'firstPartitionIsDepthZero' = false, set parentIndexCount = 0.
										Note: if parent indices are given, the first one must be -1, and *only* that index may be negative.  That is, there may be only one depth-0 mesh and it must be the first mesh.
	*/
	virtual bool						buildExplicitHierarchicalMesh
	(
		NxExplicitHierarchicalMesh& hMesh,
		const NxExplicitRenderTriangle* meshTriangles,
		physx::PxU32 meshTriangleCount,
		const NxExplicitSubmeshData* submeshData,
		physx::PxU32 submeshCount,
		physx::PxU32* meshPartition = NULL,
		physx::PxU32 meshPartitionCount = 0,
		physx::PxI32* parentIndices = NULL,
		physx::PxU32 parentIndexCount = 0
	) = 0;

	/**
		Splits the chunk in chunk[0], forming a hierarchy of fractured chunks in chunks[1...] using
		slice-mode fracturing.

		\param meshProcessingParams			describes generic mesh processing directives
		\param desc							describes the slicing surfaces (see NxFractureSliceDesc)
		\param collisionDesc				convex hulls will be generated for each chunk using the method  See NxCollisionDesc.
		\param exportCoreMesh				if true, the core mesh will be included (at depth 1) in the hierarchically split mesh.  Otherwise, it will only be used to create a hollow space.
		\param coreMeshImprintSubmeshIndex	if this is < 0, use the core mesh materials (was applyCoreMeshMaterialToNeighborChunks).  Otherwise, use the given submesh
		\param randomSeed					seed for the random number generator, to ensure reproducibility.
		\param progressListener				The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param cancel						if not NULL and *cancel is set to true, the root mesh will be restored to its original state, and the function will return at its earliest opportunity.  Meant to be set from another thread.

		\return returns true if successful.
	*/
	virtual bool							createHierarchicallySplitMesh
	(
	    const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	    const FractureTools::NxFractureSliceDesc& desc,
	    const physx::NxCollisionDesc& collisionDesc,
	    bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
	    physx::PxU32 randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	) = 0;

	/**
		Splits the mesh in chunk[0], forming a hierarchy of fractured meshes in chunks[1...] using
		cutout-mode (chippable) fracturing.

		\param meshProcessingParams		describes generic mesh processing directives
		\param desc						describes the slicing surfaces (see NxFractureCutoutDesc)
		\param cutoutSet				the cutout set to use for fracturing (see NxCutoutSet)
		\param sliceDesc				used if desc.chunkFracturingMethod = SliceFractureCutoutChunks
		\param voronoiDesc				used if desc.chunkFracturingMethod = VoronoiFractureCutoutChunks
		\param collisionDesc			convex hulls will be generated for each chunk using the method  See NxCollisionDesc.
		\param randomSeed				seed for the random number generator, to ensure reproducibility.
		\param progressListener			The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param cancel					if not NULL and *cancel is set to true, the root mesh will be restored to its original state, and the function will return at its earliest opportunity.  Meant to be set from another thread.

		\return							returns true if successful.
	*/
	virtual bool							createChippedMesh
	(
	    const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	    const FractureTools::NxFractureCutoutDesc& desc,
	    const FractureTools::NxCutoutSet& cutoutSet,
	    const FractureTools::NxFractureSliceDesc& sliceDesc,
		const FractureTools::NxFractureVoronoiDesc& voronoiDesc,
	    const physx::NxCollisionDesc& collisionDesc,
	    physx::PxU32 randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	) = 0;

	/**
		Builds an internal cutout set.

		\param pixelBuffer		pointer to be beginning of the pixel buffer
		\param bufferWidth		the width of the buffer in pixels
		\param bufferHeight		the height of the buffer in pixels
		\param snapThreshold	the pixel distance at which neighboring cutout vertices and segments may be fudged into alignment.
		\param periodic			whether or not to use periodic boundary conditions when creating cutouts from the map
	*/
	virtual void							buildCutoutSet
	(
	    const physx::PxU8* pixelBuffer,
	    physx::PxU32 bufferWidth,
	    physx::PxU32 bufferHeight,
	    physx::PxF32 snapThreshold,
		bool periodic
	) = 0;

	/**
		Calculate the mapping between a cutout fracture map and a given triangle.
		The result is a 3 by 3 matrix M composed by an affine transformation and a rotation, we can get the 3-D projection for a texture coordinate pair (u,v) with such a formula:
		(x,y,z) = M*PxVec3(u,v,1)

		\param mapping				resulted mapping, composed by an affine transformation and a rotation
		\param triangle				triangle
	**/
	virtual bool							calculateCutoutUVMapping
	(
		physx::PxMat33& mapping,
		const physx::NxExplicitRenderTriangle& triangle
	) = 0;

	/**
		Uses the passed-in target direction to find the best triangle in the root mesh with normal near the given targetDirection.  If triangles exist
		with normals within one degree of the given target direction, then one with the greatest area of such triangles is used.  Otherwise, the triangle
		with normal closest to the given target direction is used.  The resulting triangle is used to calculate a UV mapping as in the function
		calculateCutoutUVMapping (above).

		The assumption is that there exists a single mapping for all triangles on a specified face, for this feature to be useful. 

		\param mapping	resulted mapping, composed by an affine transformation and a rotation
		\param			targetDirection: the target face's normal
	**/
	virtual bool							calculateCutoutUVMapping
	(
		physx::PxMat33& mapping,
		const physx::PxVec3& targetDirection
	) = 0;

	/**
		Splits the mesh in chunk[0], forming fractured pieces chunks[1...] using
		Voronoi decomposition fracturing.

		\param meshProcessingParams			describes generic mesh processing directives
		\param desc							describes the voronoi splitting parameters surfaces (see NxFractureVoronoiDesc)
		\param collisionDesc				convex hulls will be generated for each chunk using the method  See NxCollisionDesc.
		\param exportCoreMesh				if true, the core mesh will be included (at depth 1) in the split mesh.  Otherwise, it will only be used to create a hollow space.
		\param coreMeshImprintSubmeshIndex	if this is < 0, use the core mesh materials (was applyCoreMeshMaterialToNeighborChunks).  Otherwise, use the given submesh
		\param randomSeed					seed for the random number generator, to ensure reproducibility.
		\param progressListener				The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param cancel						if not NULL and *cancel is set to true, the root mesh will be restored to its original state, and the function will return at its earliest opportunity.  Meant to be set from another thread.

		\return								returns true if successful.
	*/
	virtual bool							createVoronoiSplitMesh
	(
		const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
		const FractureTools::NxFractureVoronoiDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
		bool exportCoreMesh,
		physx::PxI32 coreMeshImprintSubmeshIndex,
		physx::PxU32 randomSeed,
		IProgressListener& progressListener,
		volatile bool* cancel = NULL
	) = 0;

	/**
		Generates a set of uniformly distributed points in the interior of the root mesh.

		\param siteBuffer			An array of PxVec3, at least the size of siteCount.
		\param siteChunkIndices		If not NULL, it must be at least the size of siteCount.
										siteCount indices will be written to this buffer,
										associating each site with a chunk that contains it.
		\param siteCount			The number of points to write into siteBuffer.
		\param randomSeed			Pointer to a seed for the random number generator, to ensure reproducibility.
										If NULL, the random number generator will not be re-seeded.
		\param microgridSize		Pointer to a grid size used for BSP creation. If NULL, the default settings will be used.
		\param meshMode				Open mesh handling.  Modes: Automatic, Closed, Open (see NxBSPOpenMode)
		\param progressListener		The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param chunkIndex			If this is a valid index, the voronoi sites will only be created within the volume of the indexed chunk.  Otherwise,
										the sites will be created within each of the root-level chunks.  Default value is an invalid index.

		\return						Returns the number of sites actually created (written to siteBuffer and siteChunkIndices).
		This may be less than the number of sites requested if site placement fails.
		*/
	virtual physx::PxU32					createVoronoiSitesInsideMesh
	(
		physx::PxVec3* siteBuffer,
		physx::PxU32* siteChunkIndices,
		physx::PxU32 siteCount,
		physx::PxU32* randomSeed,
		physx::PxU32* microgridSize,
		NxBSPOpenMode::Enum meshMode,
		IProgressListener& progressListener,
		physx::PxU32 chunkIndex = 0xFFFFFFFF
	) = 0;

	/**
		Creates scatter mesh sites randomly distributed on the mesh.

		\param meshIndices							user-allocated array of size scatterMeshInstancesBufferSize which will be filled in by this function, giving the scatter mesh index used
		\param relativeTransforms					user-allocated array of size scatterMeshInstancesBufferSize which will be filled in by this function, giving the chunk-relative transform for each chunk instance
		\param chunkMeshStarts						user-allocated array which will be filled in with offsets into the meshIndices and relativeTransforms array.
														For a chunk indexed by i, the corresponding range [chunkMeshStart[i], chunkMeshStart[i+1]-1] in meshIndices and relativeTransforms is used.
														*NOTE*: chunkMeshStart array must be of at least size N+1, where N is the number of chunks in the base explicit hierarchical mesh.
		\param scatterMeshInstancesBufferSize		the size of meshIndices and relativeTransforms array.
		\param targetChunkCount						how many chunks are in the array targetChunkIndices
		\param targetChunkIndices					an array of chunk indices which are candidates for scatter meshes.  The elements in the array chunkIndices will come from this array
		\param randomSeed							pointer to a seed for the random number generator, to ensure reproducibility.  If NULL, the random number generator will not be re-seeded.
		\param scatterMeshAssetCount				the number of different scatter meshes (not instances).  Should not exceed 255.  If scatterMeshAssetCount > 255, only the first 255 will be used.
		\param scatterMeshAssets					an array of size scatterMeshAssetCount, of the render mesh assets which will be used for the scatter meshes
		\param minCount								an array of size scatterMeshAssetCount, giving the minimum number of instances to place for each mesh
		\param maxCount								an array of size scatterMeshAssetCount, giving the maximum number of instances to place for each mesh
		\param minScales							an array of size scatterMeshAssetCount, giving the minimum scale to apply to each scatter mesh
		\param maxScales							an array of size scatterMeshAssetCount, giving the maximum scale to apply to each scatter mesh
		\param maxAngles							an array of size scatterMeshAssetCount, giving a maximum deviation angle (in degrees) from the surface normal to apply to each scatter mesh

		\return										return value: the number of instances placed in indices and relativeTransforms (will not exceed scatterMeshInstancesBufferSize)
	*/
	virtual physx::PxU32					createScatterMeshSites
	(
		physx::PxU8*						meshIndices,
		physx::PxMat44*						relativeTransforms,
		physx::PxU32*						chunkMeshStarts,
		physx::PxU32						scatterMeshInstancesBufferSize,
		physx::PxU32						targetChunkCount,
		const physx::PxU16*					targetChunkIndices,
		physx::PxU32*						randomSeed,
		physx::PxU32						scatterMeshAssetCount,
		physx::NxRenderMeshAsset**			scatterMeshAssets,
		const physx::PxU32*					minCount,
		const physx::PxU32*					maxCount,
		const physx::PxF32*					minScales,
		const physx::PxF32*					maxScales,
		const physx::PxF32*					maxAngles
	) = 0;

	/**
		Utility to visualize Voronoi cells for a given set of sites.

		\param debugRender			rendering object which will receive the drawing primitives associated with this cell visualization
		\param sites				an array of Voronoi cell sites, of length siteCount
		\param siteCount			the number of Voronoi cell sites (length of sites array)
		\param cellColors			an optional array of colors (see NxApexRenderDebug for format) for the cells.  If NULL, the white (0xFFFFFFFF) color will be used.
										If not NULL, this (of length cellColorCount) is used to color the cell graphics.  The number cellColorCount need not match siteCount.  If
										cellColorCount is less than siteCount, the cell colors will cycle.  That is, site N gets cellColor[N%cellColorCount].
		\param cellColorCount		the number of cell colors (the length of cellColors array)
		\param bounds				defines an axis-aligned bounding box which clips the visualization, since some cells extend to infinity
		\param cellIndex			if this is a valid index (cellIndex < siteCount), then only the cell corresponding to sites[cellIndex] will be drawn.  Otherwise, all cells will be drawn.
	*/
	virtual void							visualizeVoronoiCells
	(
		physx::NxApexRenderDebug& debugRender,
		const physx::PxVec3* sites,
		physx::PxU32 siteCount,
		const physx::PxU32* cellColors,
		physx::PxU32 cellColorCount,
		const physx::PxBounds3& bounds,
		physx::PxU32 cellIndex = 0xFFFFFFFF
	) = 0;

	/**
		Splits the chunk in chunk[chunkIndex], forming a hierarchy of fractured chunks using
		slice-mode fracturing.  The chunks will be rearranged so that they are in breadth-first order.

		\param chunkIndex				index of chunk to be split
		\param meshProcessingParams		describes generic mesh processing directives
		\param desc						describes the slicing surfaces (see NxFractureSliceDesc)
		\param collisionDesc			convex hulls will be generated for each chunk using the method.  See NxCollisionDesc.
		\param randomSeed				pointer to a seed for the random number generator, to ensure reproducibility.  If NULL, the random number generator will not be re-seeded.
		\param progressListener			The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param cancel					if not NULL and *cancel is set to true, the root mesh will be restored to its original state, and the function will return at its earliest opportunity.  Meant to be set from another thread.

		\return							returns true if successful.
	*/
	virtual bool							hierarchicallySplitChunk
	(
		physx::PxU32 chunkIndex,
	    const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	    const FractureTools::NxFractureSliceDesc& desc,
	    const physx::NxCollisionDesc& collisionDesc,
	    physx::PxU32* randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	) = 0;

	/**
		Splits the chunk in chunk[chunkIndex], forming fractured chunks using
		Voronoi decomposition fracturing.  The chunks will be rearranged so that they are in breadth-first order.

		\param chunkIndex				index of chunk to be split
		\param meshProcessingParams		describes generic mesh processing directives
		\param desc						describes the voronoi splitting parameters surfaces (see NxFractureVoronoiDesc)
		\param collisionDesc			convex hulls will be generated for each chunk using the method.  See NxCollisionDesc.
		\param randomSeed				pointer to a seed for the random number generator, to ensure reproducibility.  If NULL, the random number generator will not be re-seeded.
		\param progressListener			The user must instantiate an IProgressListener, so that this function may report progress of this operation
		\param cancel					if not NULL and *cancel is set to true, the root mesh will be restored to its original state, and the function will return at its earliest opportunity.  Meant to be set from another thread.

		\return returns true if successful.
	*/
	virtual bool							voronoiSplitChunk
	(
		physx::PxU32 chunkIndex,
	    const FractureTools::NxMeshProcessingParameters& meshProcessingParams,
	    const FractureTools::NxFractureVoronoiDesc& desc,
		const physx::NxCollisionDesc& collisionDesc,
	    physx::PxU32* randomSeed,
	    IProgressListener& progressListener,
	    volatile bool* cancel = NULL
	) = 0;

	/**
		Set the tolerances used in CSG calculations.

		\param linearTolerance		relative (to mesh size) tolerance used with angularTolerance to determine coplanarity.  Default = 1.0e-4.
		\param angularTolerance		used with linearTolerance to determine coplanarity.  Default = 1.0e-3
		\param baseTolerance		relative (to mesh size) tolerance used for spatial partitioning
		\param clipTolerance		relative (to mesh size) tolerance used when clipping triangles for CSG mesh building operations.  Default = 1.0e-4.
		\param cleaningTolerance	relative (to mesh size) tolerance used when cleaning the out put mesh generated from the toMesh() function.  Default = 1.0e-7.
	*/
	virtual void							setBSPTolerances
	(
		physx::PxF32 linearTolerance,
		physx::PxF32 angularTolerance,
		physx::PxF32 baseTolerance,
		physx::PxF32 clipTolerance,
		physx::PxF32 cleaningTolerance
	) = 0;

	/**
		Set the parameters used in BSP building operations.

		\param logAreaSigmaThreshold	At each step in the tree building process, the surface with maximum triangle area is compared
											to the other surface triangle areas.  If the maximum area surface is far from the "typical" set of
											surface areas, then that surface is chosen as the next splitting plane.  Otherwise, a random
											test set is chosen and a winner determined based upon the weightings below.
											The value logAreaSigmaThreshold determines how "atypical" the maximum area surface must be to
											be chosen in this manner.
											Default value = 2.0.
		\param testSetSize				Larger values of testSetSize may find better BSP trees, but will take more time to create.
											testSetSize = 0 is treated as infinity (all surfaces will be tested for each branch).
											Default value = 10.
		\param splitWeight				How much to weigh the relative number of triangle splits when searching for a BSP surface.
											Default value = 0.5.
		\param imbalanceWeight			How much to weigh the relative triangle imbalance when searching for a BSP surface.
											Default value = 0.0.
	*/
	virtual void	setBSPBuildParameters
	(
		physx::PxF32 logAreaSigmaThreshold,
		physx::PxU32 testSetSize,
		physx::PxF32 splitWeight,
		physx::PxF32 imbalanceWeight
	) = 0;


	/**
		Instantiates an NxExplicitHierarchicalMesh::NxConvexHull

		See the NxConvexHull API for its functionality.  Can be used to author chunk hulls in the
		cookChunks function.

		Use NxConvexHull::release() to delete the object.
	*/
	virtual NxExplicitHierarchicalMesh::NxConvexHull*	createExplicitHierarchicalMeshConvexHull() = 0;

	/**
		Builds a mesh used for slice fracturing, given the noise parameters and random seed.  This function is mostly intended
		for visualization - to give the user a "typical" slice surface used for fracturing.

		\return Returns the head of an array of NxExplicitRenderTriangles, of length given by the return value.
	*/
	virtual physx::PxU32					buildSliceMesh(const NxExplicitRenderTriangle*& mesh, const FractureTools::NxNoiseParameters& noiseParameters, const physx::PxPlane& slicePlane, physx::PxU32 randomSeed) = 0;

	/**
		Serialization/deserialization of the data associated with the fracture API.  This includes
		the root mesh, core mesh, and cutout set.
	*/
	virtual void							serializeFractureToolState(physx::general_PxIOStream2::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding) const = 0;
	virtual	void							deserializeFractureToolState(physx::general_PxIOStream2::PxFileBuf& stream, physx::NxExplicitHierarchicalMesh::NxEmbedding& embedding) = 0;

	/**
		Set current depth for chunk overlaps calculations.
	*/
	virtual void							setChunkOverlapsCacheDepth(physx::PxI32 depth = -1) = 0;

	/**
		Gets the NxRenderMeshAsset associated with this asset.
	*/
	virtual const NxRenderMeshAsset*		getRenderMeshAsset() const = 0;

	/**
		Set the NxRenderMeshAsset associated with this asset.
		This is the asset used for non-instanced rendering.
		NULL is a valid argument, and can be used to clear the internal mesh data.

		\return Returns true if successful.
	*/
	virtual bool							setRenderMeshAsset(NxRenderMeshAsset*) = 0;

	/**
		Set the NxRenderMeshAssets used for scatter mesh rendering associated with this asset.
		These assets will be rendered using instanced rendering.
		The array cannot contain NULL elements, if an array size greater than zero is specified.
		
		\return Returns true if successful.
	*/
	virtual bool							setScatterMeshAssets(NxRenderMeshAsset** scatterMeshAssetArray, physx::PxU32 scatterMeshAssetArraySize) = 0;

	/** Retrieve the number of scatter mesh assets */
	virtual physx::PxU32					getScatterMeshAssetCount() const = 0;

	/** Retrieve the scatter mesh asset array */
	virtual NxRenderMeshAsset* const *				getScatterMeshAssets() const = 0;

	/**
		Get the number of instanced chunk meshes in this asset.
	*/
	virtual physx::PxU32					getInstancedChunkMeshCount() const = 0;

	/**
		Set the parameters used for runtime destruction behavior.  See NxDestructibleParameters.
	*/
	virtual void                            setDestructibleParameters(const NxDestructibleParameters&) = 0;

	/**
		The NxDestructibleParameters which describe the default fracturing behavior for instanced
		NxDestructibleActors.  These may be overridden by calling setDestructibleParameters().
	*/
	virtual NxDestructibleParameters		getDestructibleParameters() const = 0;

	/**
		Set the parameters used for default destructible initialization.  See NxDestructibleInitParameters.
	*/
	virtual void                            setDestructibleInitParameters(const NxDestructibleInitParameters&) = 0;

	/**
		The parameters used for default destructible initialization.  See NxDestructibleInitParameters.
	*/
	virtual NxDestructibleInitParameters	getDestructibleInitParameters() const = 0;

	/**
		Set the name of the emitter to use when generating crumble particles.
	*/
	virtual void                            setCrumbleEmitterName(const char*) = 0;

	/**
		Set the name of the emitter to use when generating fracture-line dust particles.
	*/
	virtual void                            setDustEmitterName(const char*) = 0;

	/**
		Set the name of the fracture pattern to use when runtime fracture is enabled.
	*/
	virtual void                            setFracturePatternName(const char*) = 0;

	/**
		Set padding used for chunk neighbor tests.  This padding is relative to the largest diagonal
		of the asset's local bounding box.
		This value must be non-negative.
		Default value = 0.001f.
	*/
	virtual void							setNeighborPadding(physx::PxF32 neighborPadding) = 0;

	/**
		Get padding used for chunk neighbor tests.  Set setNeighborPadding().
	*/
	virtual physx::PxF32					getNeighborPadding() const = 0;

	/**
		Once the internal NxExplicitHierarchicalMesh is built using the fracture tools functions
		and all emitter names and parameters set, this functions builds the destructible asset.
		Every chunk (corresponding to a part in the NxExplicitHierarchicalMesh) must have
		destruction-specific data set through the descriptor passed into this function.  See
		NxDestructibleAssetCookingDesc.

		\param cookingDesc				cooking setup
		\param cacheOverlaps			whether the chunk overlaps up to chunkOverlapCacheDepth should be cached in this call
		\param chunkIndexMapUser2Apex	optional user provided PxU32 array that will contains the mapping from user chunk indices (referring to chunks in cookingDesc)
											to Apex internal chunk indices (referring to chunk is internal chunk array)
		\param chunkIndexMapApex2User	same as chunkIndexMapUser2Apex, but opposite direction
		\param chunkIndexMapCount		size of the user provided mapping arrays
	*/
	virtual void                            cookChunks(	const NxDestructibleAssetCookingDesc& cookingDesc, bool cacheOverlaps = true,
														PxU32* chunkIndexMapUser2Apex = NULL, PxU32* chunkIndexMapApex2User = NULL, PxU32 chunkIndexMapCount = 0) = 0;

	/**
		The scale factor used to apply an impulse force along the normal of chunk when fractured.  This is used
		in order to "push" the pieces out as they fracture.
	*/
	virtual physx::PxF32					getFractureImpulseScale() const = 0;

	/**
		Large impact force may be reported if rigid bodies are spawned inside one another.  In this case the realative velocity of the two
		objects will be low.  This variable allows the user to set a minimum velocity threshold for impacts to ensure that the objects are
		moving at a min velocity in order for the impact force to be considered.
	*/
	virtual physx::PxF32					getImpactVelocityThreshold() const = 0;

	/**
		The total number of chunks in the cooked asset.
	*/
	virtual physx::PxU32					getChunkCount() const = 0;

	/**
		The total number of fracture hierarchy depth levels in the cooked asset.
	*/
	virtual physx::PxU32					getDepthCount() const = 0;

	/**
		Returns the number of children for the given chunk.
		chunkIndex must be less than getChunkCount().  If it is not, this function returns 0.
	*/
	virtual physx::PxU32					getChunkChildCount(physx::PxU32 chunkIndex) const = 0;

	/**
		Returns the index for the given child of the given chunk.
		chunkIndex must be less than getChunkCount() and childIndex must be less than getChunkChildCount(chunkIndex).
		If either of these conditions is not met, the function returns NxModuleDestructibleConst::INVALID_CHUNK_INDEX.
	*/
	virtual physx::PxI32					getChunkChild(physx::PxU32 chunkIndex, physx::PxU32 childIndex) const = 0;

	/**
		If this chunk is instanced within the same asset, then this provides the instancing position offset.
		Otherwise, this function returns (0,0,0).
	*/
	virtual physx::PxVec3					getChunkPositionOffset(physx::PxU32 chunkIndex) const = 0;

	/**
		If this chunk is instanced within the same asset, then this provides the instancing UV offset.
		Otherwise, this function returns (0,0).
	*/
	virtual physx::PxVec2					getChunkUVOffset(physx::PxU32 chunkIndex) const = 0;

	/**
		The render mesh asset part index associated with this chunk.
	*/
	virtual physx::PxU32					getPartIndex(physx::PxU32 chunkIndex) const = 0;

	/**
		Trim collision geometry to prevent explosive behavior.  maxTrimFraction is the maximum (relative) distance to trim a hull in the direction
		of each trim plane.
		
		\return Returns true iff successful.
	*/
	virtual void							trimCollisionGeometry(const physx::PxU32* partIndices, physx::PxU32 partIndexCount, physx::PxF32 maxTrimFraction = 0.2f) = 0;

	/**
		Returns stats (sizes, counts) for the asset.  See NxDestructibleAssetStats.
	*/
	virtual void							getStats(NxDestructibleAssetStats& stats) const = 0;

	/**
		Ensures that the asset has chunk overlap information cached up to the given depth.
		If depth < 0 (as it is by default), the depth will be taken to be the supportDepth
		given in the asset's destructibleParameters.
		It is ok to pass in a depth greater than any chunk depth in the asset.
	*/
	virtual void							cacheChunkOverlapsUpToDepth(physx::PxI32 depth = -1) = 0;

	/**
		Clears the chunk overlap cache.

		\param depth			Depth to be cleared. -1 for all depths.
		\param keepCachedFlag	If the flag is set, the depth is considered to be cached even if the overlaps list is empty.
	*/
	virtual void							clearChunkOverlaps(physx::PxI32 depth = -1, bool keepCachedFlag = false) = 0;

	/**
		Adds edges to the support graph. Edges must connect chunks of equal depth.
		The indices refer to the reordered chunk array, a mapping is provided in cookChunks.
	*/
	virtual void							addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges) = 0;

	/**
		Removes edges from support graph.
		The indices refer to the reordered chunk array, a mapping is provided in cookChunks.

		\param supportGraphEdges		Integer pairs representing indices to chunks that are linked
		\param numSupportGraphEdges		Number of provided integer pairs.
		\param keepCachedFlagIfEmpty	If the flag is set, the depth is considered to be cached even if the overlaps list is empty.
	*/
	virtual void							removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty) = 0;

	/**
		The size of the array returned by getCachedOverlapsAtDepth(depth) (see below).
		Note: this function will not trigger overlap caching for the given depth.  If no overlaps
		have been calculated for the depth given, this function returns NULL.
	*/
	virtual physx::PxU32					getCachedOverlapCountAtDepth(physx::PxU32 depth) = 0;

	/**
		Array of integer pairs, indexing chunk pairs which touch at a given depth in the heirarcy.
		The size of the array is given by getCachedOverlapCountAtDepth(depth).
		Note: this function will not trigger overlap caching for the given depth.  If no overlaps
		have been calculated for the depth given, this function returns NULL.
	*/
	virtual const NxIntPair*				getCachedOverlapsAtDepth(physx::PxU32 depth) = 0;

	/**
	\brief Apply a transformation to destructible asset

	This is a permanent transformation and it changes the object state. Should only be called immediately before serialization
	and without further modifying the object later on.

	\param transformation	This matrix is allowed to contain a translation and a rotation
	\param scale			Apply a uniform scaling as well
	*/
	virtual void							applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale) = 0;

	/**
	\brief Apply an arbitrary affine transformation to destructible asset

	This is a permanent transformation and it changes the object state. Should only be called immediately before serialization
	and without further modifying the object later on.

	\param transformation	This matrix is allowed to contain translation, rotation, scale and skew
	*/
	virtual void							applyTransformation(const physx::PxMat44& transformation) = 0;

	/**
	\brief Set a maximum fracture depth for a given platform string

	Returns true if the supplied maxDepth is lesser than the number of chunk depth levels for this asset
	*/
	virtual bool                            setPlatformMaxDepth(NxPlatformTag platform, physx::PxU32 maxDepth) = 0;

	/**
	\brief Removes the maximum fracture depth limit for a given platform string

	Returns true if the platform's maximum fracture depth was previously set and now removed
	*/
	virtual bool                            removePlatformMaxDepth(NxPlatformTag platform) = 0;

	/**
	\brief Returns the size of the actor transform array.  See getActorTransforms() for a description of this data.
	*/
	virtual physx::PxU32					getActorTransformCount() const = 0;

	/**
	\brief Returns the head of the actor transform array.  This list is a convenience for placing actors in a level from poses authored in a level editor.
	The transforms may contain scaling.
	*/
	virtual const physx::PxMat44*			getActorTransforms() const = 0;

	/**
	\brief Append transforms to the actor transform list.  See getActorTransforms() for a description of this data.

	\param transforms		Head of an array of transforms
	\param transformCount	Size of transforms
	*/
	virtual void							appendActorTransforms(const physx::PxMat44* transforms, physx::PxU32 transformCount) = 0;

	/**
	\brief Clear the actor transform array.  See getActorTransforms() for a description of this data.
	*/
	virtual void							clearActorTransforms() = 0;
};

/**
	Destructible asset API.  Destructible actors are instanced from destructible assets.
*/
class NxDestructibleAsset : public NxApexAsset
{
public:
	enum ChunkFlags
	{
		ChunkEnvironmentallySupported =		(1 << 0),
		ChunkAndDescendentsDoNotFracture =	(1 << 1),
		ChunkDoesNotFracture =				(1 << 2),
		ChunkDoesNotCrumble =				(1 << 3),
#if APEX_RUNTIME_FRACTURE
		ChunkRuntimeFracture =				(1 << 4),
#endif
		ChunkIsInstanced =					(1 << 16)
	};

	/** Instancing */

	/**
		Instances the NxDestructibleAsset, creating an NxDestructibleActor, using the NxDestructibleActorDesc.
		See NxDestructibleActor and NxDestructibleActorDesc.  This asset will own the NxDestructibleActor,
		so that any NxDestructibleActor created by it will be released when this asset is released.
		You may also call the NxDestructibleActor's release() method at any time.
	*/
	virtual void							releaseDestructibleActor(NxDestructibleActor& actor) = 0;


	/** General */

	/**
		Create a destructible actor representing the destructible asset in a scene. 
		Unlike a call to createApexActor, here the created actor takes explicit ownership of the provided actorParams.
		This can represent either the destructible descriptor or previously serialized destructible state.
		Note: The client should not attempt to use the provided actorParams after calling this method.
	*/
	virtual NxDestructibleActor*			createDestructibleActorFromDeserializedState(::NxParameterized::Interface* actorParams, NxApexScene& apexScene) = 0;

	/**
		The NxDestructibleParameters which describe the default fracturing behavior for instanced
		NxDestructibleActors.
	*/
	virtual NxDestructibleParameters		getDestructibleParameters() const = 0;

	/**
		The parameters used for default destructible initialization.  See NxDestructibleInitParameters.
	*/
	virtual NxDestructibleInitParameters	getDestructibleInitParameters() const = 0;

	/**
		The name of the emitter to use when generating crumble particles.
		Returns NULL if no emitter is configured.
	*/
	virtual const char*						getCrumbleEmitterName() const = 0;

	/**
		The name of the emitter to use when generating fracture-line dust particles.
		Returns NULL if no emitter is configured.
	*/
	virtual const char*						getDustEmitterName() const = 0;

	/**
		The total number of chunks in the asset.
	*/
	virtual physx::PxU32					getChunkCount() const = 0;

	/**
		The total number of fracture hierarchy depth levels in the asset.
	*/
	virtual physx::PxU32					getDepthCount() const = 0;

	/**
		Gets the NxRenderMeshAsset associated with this asset.
	*/
	virtual const NxRenderMeshAsset*		getRenderMeshAsset() const = 0;

	/**
		Set the NxRenderMeshAsset associated with this asset.
		This is the asset used for non-instanced rendering.
		NULL is a valid argument, and can be used to clear the internal mesh data.
		Returns true if successful.
	*/
	virtual bool							setRenderMeshAsset(NxRenderMeshAsset*) = 0;

	/** Retrieve the number of scatter mesh assets */
	virtual physx::PxU32					getScatterMeshAssetCount() const = 0;

	/** Retrieve the scatter mesh asset array */
	virtual NxRenderMeshAsset* const *				getScatterMeshAssets() const = 0;

	/**
		Get the number of instanced chunk meshes in this asset.
	*/
	virtual physx::PxU32					getInstancedChunkMeshCount() const = 0;

	/**
		Returns stats (sizes, counts) for the asset.  See NxDestructibleAssetStats.
	*/
	virtual void							getStats(NxDestructibleAssetStats& stats) const = 0;

	/**
		Ensures that the asset has chunk overlap information cached up to the given depth.
		If depth < 0 (as it is by default), the depth will be taken to be the supportDepth
		given in the asset's destructibleParameters.
		It is ok to pass in a depth greater than any chunk depth in the asset.
	*/
	virtual void							cacheChunkOverlapsUpToDepth(physx::PxI32 depth = -1) = 0;

	/**
		Clears the chunk overlap cache.
		If depth < 0 (as it is by default), it clears the cache for each depth.

		\param depth			Depth to be cleared. -1 for all depths.
		\param keepCachedFlag	If the flag is set, the depth is considered to be cached even if the overlaps list is empty.
	*/
	virtual void							clearChunkOverlaps(physx::PxI32 depth = -1, bool keepCachedFlag = false) = 0;

	/**
		Adds edges to the support graph. Edges must connect chunks of equal depth.
		The indices refer to the reordered chunk array, a mapping is provided in cookChunks.

		\param supportGraphEdges		Integer pairs representing indices to chunks that are linked
		\param numSupportGraphEdges		Number of provided integer pairs.
	*/
	virtual void							addChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges) = 0;

	/**
		Removes edges from support graph.
		The indices refer to the reordered chunk array, a mapping is provided in cookChunks.

		\param supportGraphEdges		Integer pairs representing indices to chunks that are linked
		\param numSupportGraphEdges		Number of provided integer pairs.
		\param keepCachedFlagIfEmpty	If the flag is set, the depth is considered to be cached even if the overlaps list is empty.
	*/
	virtual void							removeChunkOverlaps(NxIntPair* supportGraphEdges, PxU32 numSupportGraphEdges, bool keepCachedFlagIfEmpty) = 0;

	/**
		The size of the array returned by getCachedOverlapsAtDepth(depth) (see below).
		Note: this function will not trigger overlap caching for the given depth.
	*/
	virtual physx::PxU32					getCachedOverlapCountAtDepth(physx::PxU32 depth) const = 0;

	/**
		Array of integer pairs, indexing chunk pairs which touch at a given depth in the hierarchy.
		The size of the array is given by getCachedOverlapCountAtDepth(depth).
		Note: this function will not trigger overlap caching for the given depth.  If no overlaps
		have been calculated for the depth given, this function returns NULL.
	*/
	virtual const NxIntPair*				getCachedOverlapsAtDepth(physx::PxU32 depth) const = 0;

	/**
		If this chunk is instanced within the same asset, then this provides the instancing position offset.
		Otherwise, this function returns (0,0,0).
	*/
	virtual physx::PxVec3					getChunkPositionOffset(physx::PxU32 chunkIndex) const = 0;

	/**
		If this chunk is instanced within the same asset, then this provides the instancing UV offset.
		Otherwise, this function returns (0,0).
	*/
	virtual physx::PxVec2					getChunkUVOffset(physx::PxU32 chunkIndex) const = 0;

	/**
		Retrieve flags (see ChunkFlags) for the given chunk.
	*/
	virtual physx::PxU32					getChunkFlags(physx::PxU32 chunkIndex) const = 0;

	/**
		Accessor to query the depth of a given chunk.
	*/
	virtual physx::PxU16					getChunkDepth(physx::PxU32 chunkIndex) const = 0;

	/**
		Returns the index of the given chunk's parent.  If the chunk has no parent (is the root of the fracture hierarchy),
		then -1 is returned.
	*/
	virtual physx::PxI32					getChunkParentIndex(physx::PxU32 chunkIndex) const = 0;

	/** 
		Returns the chunk bounds in the asset (local) space.
	*/
	virtual physx::PxBounds3 getChunkActorLocalBounds(physx::PxU32 chunkIndex) const = 0;

	/**
		The render mesh asset part index associated with this chunk.
	*/
	virtual physx::PxU32					getPartIndex(physx::PxU32 chunkIndex) const = 0;

	/**
		The number of convex hulls associated with a given part.
	*/
	virtual physx::PxU32					getPartConvexHullCount(const physx::PxU32 partIndex) const = 0;

	/**
		Returns the head of an array to convex hull NxParamterized::Interface* pointers for a given part.
	*/
	virtual NxParameterized::Interface**	getPartConvexHullArray(const physx::PxU32 partIndex) const = 0;

	/**
	\brief Returns the size of the actor transform array.  See getActorTransforms() for a description of this data.
	*/
	virtual physx::PxU32					getActorTransformCount() const = 0;

	/**
	\brief Returns the head of the actor transform array.  This list is a convenience for placing actors in a level from poses authored in a level editor.
	The transforms may contain scaling.
	*/
	virtual const physx::PxMat44*			getActorTransforms() const = 0;

	/**
	\brief Apply a transformation to destructible asset

	This is a permanent transformation and it changes the object state. Should only be called immediately before serialization
	and without further modifying the object later on.

	\param transformation	This matrix is allowed to contain a translation and a rotation
	\param scale			Apply a uniform scaling as well
	*/
	virtual void							applyTransformation(const physx::PxMat44& transformation, physx::PxF32 scale) = 0;

	/**
	\brief Apply an arbitrary affine transformation to destructible asset

	This is a permanent transformation and it changes the object state. Should only be called immediately before serialization
	and without further modifying the object later on.

	\param transformation	This matrix is allowed to contain translation, rotation, scale and skew
	*/
	virtual void							applyTransformation(const physx::PxMat44& transformation) = 0;

	/**
		Rebuild the collision volumes for the given chunk, using the geometryDesc (see NxDestructibleGeometryDesc).
		Returns true iff successful.
	*/
	virtual bool							rebuildCollisionGeometry(physx::PxU32 partIndex, const NxDestructibleGeometryDesc& geometryDesc) = 0;

protected:
	/** Hidden destructor.  Use release(). */
	virtual									~NxDestructibleAsset() {}
};


#if !defined(PX_PS4)
	#pragma warning(pop)
#endif	//!PX_PS4

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_DESTRUCTIBLE_ASSET_H
