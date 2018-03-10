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


#ifndef PX_PHYSICS_SCB_NPDEPS
#define PX_PHYSICS_SCB_NPDEPS

namespace physx
{

// The Scb layer needs to delete the owning Np objects, but we don't want to include the Np headers
// necessary to find their addresses. So we use link-level dependencies instead.

namespace Scb
{
	class Base;
	class Shape;
	class RigidObject;
	class Constraint;
	class Scene;
	class ArticulationJoint;
	class Articulation;
	class RigidStatic;
	class Body;
}

namespace Sc
{
	class RigidCore;
}

class PxScene;

extern void NpDestroy(Scb::Base&);

// we want to get the pointer to the rigid object that owns a shape, and the two actor pointers for a constraint, so that we don't
// duplicate the scene graph in Scb

extern PxU32 NpRigidStaticGetShapes(Scb::RigidStatic& rigid, void* const *&shapes);
extern PxU32 NpRigidDynamicGetShapes(Scb::Body& body, void* const *&shapes);
extern size_t NpShapeGetScPtrOffset();
extern void NpShapeIncRefCount(Scb::Shape& shape);
extern void NpShapeDecRefCount(Scb::Shape& shape);

extern Sc::RigidCore* NpShapeGetScRigidObjectFromScbSLOW(const Scb::Shape &);
extern void NpConstraintGetRigidObjectsFromScb(const Scb::Constraint&, Scb::RigidObject*&, Scb::RigidObject*&);
extern void NpArticulationJointGetBodiesFromScb(Scb::ArticulationJoint&, Scb::Body*&, Scb::Body*&);
extern Scb::Body* NpArticulationGetRootFromScb(Scb::Articulation&);

// get an Np scene from the Scb scene, so that we can store the latter in the Scb objects
extern PxScene* NpGetPxScene(Scb::Scene& scene);

}

#endif
