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
#include "MinPhysxSdkVersion.h"
#if NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED

#include "NxApex.h"
#include "ModuleDestructible.h"
#include "DestructibleActorJoint.h"
#include "DestructibleScene.h"
#include "NxDestructibleActorJoint.h"
#include "DestructibleActorProxy.h"

#if NX_SDK_VERSION_MAJOR == 2
#include <NxCylindricalJoint.h>
#include <NxCylindricalJointDesc.h>
#include <NxD6Joint.h>
#include <NxD6JointDesc.h>
#include <NxDistanceJoint.h>
#include <NxDistanceJointDesc.h>
#include <NxFixedJoint.h>
#include <NxFixedJointDesc.h>
#include <NxPointInPlaneJoint.h>
#include <NxPointInPlaneJointDesc.h>
#include <NxPointOnLineJoint.h>
#include <NxPointOnLineJointDesc.h>
#include <NxPrismaticJoint.h>
#include <NxPrismaticJointDesc.h>
#include <NxPulleyJoint.h>
#include <NxPulleyJointDesc.h>
#include <NxRevoluteJoint.h>
#include <NxRevoluteJointDesc.h>
#include <NxScene.h>
#include <NxSphericalJoint.h>
#include <NxSphericalJointDesc.h>
#elif NX_SDK_VERSION_MAJOR == 3
#include <PxScene.h>
#include <PxJoint.h>
#include <PxD6Joint.h>
#include <PxDistanceJoint.h>
#include <PxFixedJoint.h>
#include <PxPrismaticJoint.h>
#include <PxRevoluteJoint.h>
#include <PxSphericalJoint.h>
#endif

namespace physx
{
namespace apex
{
namespace destructible
{

DestructibleActorJoint::DestructibleActorJoint(const NxDestructibleActorJointDesc& desc, DestructibleScene& dscene) :
	joint(NULL)
{
	if (desc.destructible[0] == NULL && desc.destructible[1] == NULL)
	{
		APEX_DEBUG_WARNING("Both destructible actors in DestructibleActorJoint are NULL.");
		return;
	}

#if NX_SDK_VERSION_MAJOR == 2
	if (desc.jointDesc == NULL)
	{
		APEX_DEBUG_WARNING("No jointDesc provided to DestructibleActorJoint.");
		return;
	}
	NxJointDesc& jointDesc = *desc.jointDesc;
#elif NX_SDK_VERSION_MAJOR == 3
	PxRigidActor*	actor[2]		= {desc.actor[0], desc.actor[1]};
	PxVec3			localAxis[2]	= {desc.localAxis[0], desc.localAxis[1]};
	PxVec3			localAnchor[2]	= {desc.localAnchor[0], desc.localAnchor[1]};
	PxVec3			localNormal[2]	= {desc.localNormal[0], desc.localNormal[1]};

	PxTransform		localFrame[2];
#endif

	for (int i = 0; i < 2; ++i)
	{
		if (desc.destructible[i] == NULL)
		{
			structure[i] = NULL;
			attachmentChunkIndex[i] = NxModuleDestructibleConst::INVALID_CHUNK_INDEX;

#if NX_SDK_VERSION_MAJOR == 3
			if(NULL == actor[i])
			{
				// World constrained			
				physx::PxMat33Legacy	rot;
				rot.setColumn(0, desc.globalAxis[i]);
				rot.setColumn(1, desc.globalNormal[i]);
				rot.setColumn(2, desc.globalAxis[i].cross(desc.globalNormal[i]));

				localFrame[i].p	= desc.globalAnchor[i];
				localFrame[i].q	= rot.toQuat();
				localFrame[i].q.normalize();
			}
			else
			{
				// Constrained to physics object
				physx::PxMat33Legacy	rot;
				rot.setColumn(0, localAxis[i]);
				rot.setColumn(1, localNormal[i]);
				rot.setColumn(2, localAxis[i].cross(localNormal[i]));

				localFrame[i].p	= localAnchor[i];
				localFrame[i].q	= rot.toQuat();
				localFrame[i].q.normalize();
			}
#endif
			continue;
		}

		NxActor* attachActor = NULL;
		DestructibleActor& destructible = ((DestructibleActorProxy*)desc.destructible[i])->impl;
		structure[i] = destructible.getStructure();

		attachmentChunkIndex[i] = desc.attachmentChunkIndex[i];

		if (attachmentChunkIndex[i] >= 0 && attachmentChunkIndex[i] < (physx::PxI32)destructible.getAsset()->getChunkCount())
		{
			DestructibleStructure::Chunk& chunk = structure[i]->chunks[destructible.getFirstChunkIndex() + attachmentChunkIndex[i]];
			attachActor = structure[i]->dscene->chunkIntact(chunk);
		}

		if (attachActor == NULL)
		{
			physx::PxF32 minDistance = PX_MAX_F32;
			for (physx::PxU32 j = 0; j < destructible.getAsset()->getChunkCount(); ++j)
			{
				DestructibleAssetParametersNS::Chunk_Type& source = destructible.getAsset()->mParams->chunks.buf[j];
				const bool hasChildren = source.numChildren != 0;
				if (!hasChildren)	// Only attaching to lowest-level chunks, initially
				{
					DestructibleStructure::Chunk& chunk = structure[i]->chunks[destructible.getFirstChunkIndex() + j];
					NxActor* actor = structure[i]->dscene->chunkIntact(chunk);
					if (actor)
					{
#if NX_SDK_VERSION_MAJOR == 2
						const physx::PxF32 distance = (actor->getGlobalPose() * chunk.localSphere.center).distance(NXFROMPXVEC3(desc.globalAnchor[i]));
#elif NX_SDK_VERSION_MAJOR == 3
						const physx::PxF32 distance = (physx::PxMat34Legacy(actor->getGlobalPose()) * chunk.localSphere.center - desc.globalAnchor[i]).magnitude();
#endif
						if (distance < minDistance)
						{
							attachActor = actor;
							attachmentChunkIndex[i] = (physx::PxI32)(destructible.getFirstChunkIndex() + j);
							minDistance = distance;
						}
					}
				}
			}
		}

		if (attachActor == NULL)
		{
			APEX_DEBUG_WARNING("No physx actor could be found in destructible actor %p to attach the joint.", desc.destructible[i]);
			return;
		}

#if NX_SDK_VERSION_MAJOR == 2
		jointDesc.actor[i] = attachActor;

		if (dscene.getModulePhysXScene() != &attachActor->getScene())
		{
			APEX_DEBUG_WARNING("Trying to joint actors from a scene different from the joint scene.");
			return;
		}

		NxMat34 globalPose = attachActor->getGlobalPose();
		globalPose.multiplyByInverseRT(NXFROMPXVEC3(desc.globalAnchor[i]), jointDesc.localAnchor[i]);
		globalPose.M.multiplyByTranspose(NXFROMPXVEC3(desc.globalAxis[i]), jointDesc.localAxis[i]);
		globalPose.M.multiplyByTranspose(NXFROMPXVEC3(desc.globalNormal[i]), jointDesc.localNormal[i]);
#elif NX_SDK_VERSION_MAJOR == 3
		actor[i] = (PxRigidActor*)attachActor;

		if (attachActor->getScene() != NULL && dscene.getModulePhysXScene() != attachActor->getScene())
		{
			APEX_DEBUG_WARNING("Trying to joint actors from a scene different from the joint scene.");
			return;
		}

		physx::PxMat34Legacy globalPose(attachActor->getGlobalPose());

		globalPose.multiplyByInverseRT(desc.globalAnchor[i], localAnchor[i]);
		globalPose.M.multiplyByTranspose(desc.globalAxis[i], localAxis[i]);
		globalPose.M.multiplyByTranspose(desc.globalNormal[i], localNormal[i]);

		physx::PxMat33Legacy	rot;
		rot.setColumn(0, localAxis[i]);
		rot.setColumn(1, localNormal[i]);
		rot.setColumn(2, localAxis[i].cross(localNormal[i]));

		localFrame[i].p	= localAnchor[i];
		localFrame[i].q	= rot.toQuat();
		localFrame[i].q.normalize();
#endif
	}

#if NX_SDK_VERSION_MAJOR == 2
	joint = dscene.getModulePhysXScene()->createJoint(jointDesc);
#elif NX_SDK_VERSION_MAJOR == 3
	dscene.getModulePhysXScene()->lockRead();
	switch (desc.type)
	{
	case PxJointConcreteType::eD6:
		joint	= PxD6JointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	case PxJointConcreteType::eDISTANCE:
		joint	= PxDistanceJointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	case PxJointConcreteType::eFIXED:
		joint	= PxFixedJointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	case PxJointConcreteType::ePRISMATIC:
		joint	= PxPrismaticJointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	case PxJointConcreteType::eREVOLUTE:
		joint	= PxRevoluteJointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	case PxJointConcreteType::eSPHERICAL:
		joint	= PxSphericalJointCreate(dscene.getModulePhysXScene()->getPhysics(), actor[0], localFrame[0], actor[1], localFrame[1]);
		break;
	default:
		PX_ALWAYS_ASSERT();
		break;
	}
	dscene.getModulePhysXScene()->unlockRead();
#endif

	PX_ASSERT(joint != NULL);
}

DestructibleActorJoint::~DestructibleActorJoint()
{
	if (joint)
	{
#if NX_SDK_VERSION_MAJOR == 2
		joint->getScene().releaseJoint(*joint);
#elif NX_SDK_VERSION_MAJOR == 3
		joint->release();
#endif
	}
}

bool DestructibleActorJoint::updateJoint()
{
	if (!joint)
	{
		return false;
	}

#if NX_SDK_VERSION_MAJOR == 2
	NxActor* actors[2];
	joint->getActors(actors, actors + 1);
#elif NX_SDK_VERSION_MAJOR == 3
	PxRigidActor* actors[2];
	joint->getActors(actors[0], actors[1]);
#endif

	bool needsReattachment = false;
	for (physx::PxU32 i = 0; i < 2; ++i)
	{
		if (structure[i] == NULL)
		{
			continue;
		}
		if (attachmentChunkIndex[i] < 0 || attachmentChunkIndex[i] >= (physx::PxI32)structure[i]->chunks.size())
		{
			return false;
		}
		DestructibleStructure::Chunk& chunk = structure[i]->chunks[(physx::PxU32)attachmentChunkIndex[i]];
		NxActor* actor = structure[i]->dscene->chunkIntact(chunk);
		if (actor == NULL)
		{
			return false;
		}
		if ((NxActor*)actors[i] != actor)
		{
			needsReattachment = true;
	#if NX_SDK_VERSION_MAJOR == 2
			actors[i] = actor;
	#elif NX_SDK_VERSION_MAJOR == 3
			actors[i] = (PxRigidActor*)actor;
	#endif		
		}

	}

	if (!needsReattachment)
	{
		return true;
	}
#if NX_SDK_VERSION_MAJOR == 2
	NxScene& scene = joint->getScene();

	// Need to re-attach to new actor
	switch (joint->getType())
	{
	case NX_JOINT_PRISMATIC:
	{
		NxPrismaticJointDesc desc;
		((NxPrismaticJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_REVOLUTE:
	{
		NxRevoluteJointDesc desc;
		((NxRevoluteJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_CYLINDRICAL:
	{
		NxCylindricalJointDesc desc;
		((NxCylindricalJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_SPHERICAL:
	{
		NxSphericalJointDesc desc;
		((NxSphericalJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_POINT_ON_LINE:
	{
		NxPointOnLineJointDesc desc;
		((NxPointOnLineJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_POINT_IN_PLANE:
	{
		NxPointInPlaneJointDesc desc;
		((NxPointInPlaneJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_DISTANCE:
	{
		NxDistanceJointDesc desc;
		((NxDistanceJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_PULLEY:
	{
		NxPulleyJointDesc desc;
		((NxPulleyJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_FIXED:
	{
		NxFixedJointDesc desc;
		((NxFixedJoint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	case NX_JOINT_D6:
	{
		NxD6JointDesc desc;
		((NxD6Joint*)joint)->saveToDesc(desc);
		desc.actor[0] = actors[0];
		desc.actor[1] = actors[1];
		scene.releaseJoint(*joint);
		joint = scene.createJoint(desc);
		break;
	}
	default:
	{
		PX_ASSERT(!"Unknown joint type.  Cannot change actor; deleting.\n");
		scene.releaseJoint(*joint);
		joint = NULL;
		break;
	}
	}
#elif NX_SDK_VERSION_MAJOR == 3
#define JOINT_SET_ACTORS_WORKAROUND	1
#if JOINT_SET_ACTORS_WORKAROUND
	NxScene* pxScenes[2] = { NULL, NULL };
	for (physx::PxU32 i = 0; i < 2; ++i)
	{
		if (actors[i] != NULL)
		{
			pxScenes[i] = actors[i]->getScene();
			if (actors[i]->getScene() != NULL)
			{
				pxScenes[i]->lockWrite();
				pxScenes[i]->removeActor(*actors[i], false);
				pxScenes[i]->unlockWrite();
			}
		}
	}
#endif
	joint->setActors(actors[0], actors[1]);
#if JOINT_SET_ACTORS_WORKAROUND
	for (physx::PxU32 i = 0; i < 2; ++i)
	{
		if (actors[i] != NULL && pxScenes[i] != NULL)
		{
			pxScenes[i]->lockWrite();
			pxScenes[i]->addActor(*actors[i]);
			pxScenes[i]->unlockWrite();
		}
	}
#endif
#endif
	return joint != NULL;
}

}
}
} // end namespace physx::apex

#endif // NX_SDK_VERSION_NUMBER >= MIN_PHYSX_SDK_VERSION_REQUIRED
