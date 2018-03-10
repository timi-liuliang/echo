/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSICS_SCP_PARTICLE_SYSTEM_SIM
#define PX_PHYSICS_SCP_PARTICLE_SYSTEM_SIM

#include "CmPhysXCommon.h"
#include "PxPhysXConfig.h"
#if PX_USE_PARTICLE_SYSTEM_API

#include "ScScene.h"
#include "ScRigidSim.h"
#include "ScActor.h"
#include "PsPool.h"

#include "ScParticlePacketShape.h"
#include "PxvParticleSystemSim.h"

namespace physx
{

class PxvParticleSystemSim;
class PxvParticleSystemState;
struct PxvParticleSystemSimDataDesc;
struct PxvParticleShapesUpdateInput;
struct PxvParticleCollisionUpdateInput;

#if PX_SUPPORT_GPU_PHYSX
class PxParticleDeviceExclusiveAccess;
#endif

namespace Sc
{
	class ParticleSystemCore;
	class ShapeSim;
	class ParticlePacketShape;

#define PX_PARTICLE_SYSTEM_DEBUG_RENDERING			1

	class ParticleSystemSim : public ActorSim
	{
	public:

		ParticleSystemSim(Scene&, ParticleSystemCore&);
		
		void					release(bool releaseStateBuffers);

		PxFilterData			getSimulationFilterData() const;
		void					scheduleRefiltering();
		void					resetFiltering();

		void					setFlags(PxU32 flags);
		PxU32					getInternalFlags() const;

		void					getSimParticleData(PxvParticleSystemSimDataDesc& simParticleData, bool devicePtr) const;
		PxvParticleSystemState&	getParticleState();

		void					addInteraction(const ParticlePacketShape& particleShape, const ShapeSim& shape, const PxU32 ccdPass);
		void					removeInteraction(const ParticlePacketShape& particleShape, const ShapeSim& shape, bool isDyingRb, const PxU32 ccdPass);
		void					onRbShapeChange(const ParticlePacketShape& particleShape, const ShapeSim& shape);

		void					processShapesUpdate();
#if PX_SUPPORT_GPU_PHYSX
		Ps::IntBool				isGpu() const { return mLLSim->isGpuV(); }
#endif		
		// batched updates
		static PxBaseTask&	scheduleShapeGeneration(InteractionScene& scene, const Ps::Array<ParticleSystemSim*>& particleSystems, PxBaseTask& continuation);
		static PxBaseTask&	scheduleDynamicsCpu(InteractionScene& scene, const Ps::Array<ParticleSystemSim*>& particleSystems, PxBaseTask& continuation);
		static PxBaseTask&	scheduleCollisionPrep(InteractionScene& scene, const Ps::Array<ParticleSystemSim*>& particleSystems, PxBaseTask& continuation);
		static PxBaseTask&	scheduleCollisionCpu(InteractionScene& scene, const Ps::Array<ParticleSystemSim*>& particleSystems, PxBaseTask& continuation);
		static PxBaseTask&	schedulePipelineGpu(InteractionScene& scene, const Ps::Array<ParticleSystemSim*>& particleSystems, PxBaseTask& continuation);

		//---------------------------------------------------------------------------------
		// Actor implementation
		//---------------------------------------------------------------------------------
	public:
		// non-DDI methods:

		// Core functionality
		void				startStep();
		void				endStep();

		void				unlinkParticleShape(ParticlePacketShape* particleShape);

		ParticleSystemCore&	getCore() const;
		
#if PX_SUPPORT_GPU_PHYSX
		void				enableDeviceExclusiveModeGpu();
		PxParticleDeviceExclusiveAccess*
							getDeviceExclusiveAccessGpu() const;
#endif

	private:
		~ParticleSystemSim() {}

		void				createShapeUpdateInput(PxvParticleShapesUpdateInput& input);	
		void				createCollisionUpdateInput(PxvParticleCollisionUpdateInput& input);	
		void				updateRigidBodies();
		void				prepareCollisionInput(PxBaseTask* continuation);

		// ParticleSystem packet handling
		void				releaseParticlePacketShapes();
		PX_INLINE void		addParticlePacket(PxvParticleShape* llParticleShape);
		PX_INLINE void		removeParticlePacket(const PxvParticleShape * llParticleShape);


#if PX_ENABLE_DEBUG_VISUALIZATION
	public:
		void visualizeStartStep(Cm::RenderOutput& out);
		void visualizeEndStep(Cm::RenderOutput& out);

	private:
		void	visualizeParticlesBounds(Cm::RenderOutput& out);
		void	visualizeParticles(Cm::RenderOutput& out);
		void	visualizeCollisionNormals(Cm::RenderOutput& out);
		void	visualizeSpatialGrid(Cm::RenderOutput& out);
		void	visualizeBroadPhaseBounds(Cm::RenderOutput& out);
		void	visualizeInteractions(Cm::RenderOutput& out);	// MS: Might be helpful for debugging
#endif  // PX_ENABLE_DEBUG_VISUALIZATION


	private:
		PxvParticleSystemSim*	mLLSim;

		// Array of particle packet shapes
		Ps::Pool<ParticlePacketShape> mParticlePacketShapePool;
		Ps::Array<ParticlePacketShape*> mParticlePacketShapes;

		// Count interactions for sizing the contact manager stream
		PxU32 mInteractionCount;

		typedef Cm::DelegateTask<Sc::ParticleSystemSim, &Sc::ParticleSystemSim::prepareCollisionInput> CollisionInputPrepTask;
		CollisionInputPrepTask mCollisionInputPrepTask;
	};

} // namespace Sc

}

#endif	// PX_USE_PARTICLE_SYSTEM_API

#endif
