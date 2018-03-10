//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        ICollision.h
// DESCRIPTION: An interface class to our collisions
// AUTHOR:      Peter Watje
// HISTORY:     3-15-00
//**************************************************************************/
#pragma once

//
// \TODO Consider these classes for a move to maxsdk/samples/objects/particles/colliders/...
//

#include "maxheap.h"
#include "iparamm2.h"
#include "iFnPub.h"

#define PLANAR_COLLISION_ID Class_ID(0x14585111, 0x444a7dcf)
#define SPHERICAL_COLLISION_ID Class_ID(0x14585222, 0x555a7dcf)
#define MESH_COLLISION_ID Class_ID(0x14585333, 0x666a7dcf)


#define COLLISION_FO_INTERFACE Class_ID(0x14585444, 0x777a7dcf)

#define GetCollisionOpsInterface(cd) \
			(CollisionOps *)(cd)->GetInterface(COLLISION_FO_INTERFACE)



#define POINT_COLLISION		1
#define SPHERE_COLLISION	2
#define BOX_COLLISION		4
#define EDGE_COLLISION		8

/*! \sa  Class CollisionOps, Class CollisionPlane, Class CollisionSphere, Class CollisionVNormal, Class CollisionMesh, Class ReferenceTarget, Class Box3,  Class Point3\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the virtual class to support collision objects. All
collision detection classes should derive from this class. The purpose of this
class is to improve the particle collision system for older particle systems
and to let other systems such as Flex and MAXScript benefit from a more open
architecture and to provide an interface to determine if a particle hit a
surface. Additionally three basic collision detection classes are provided, a
planar, a spherical, and a mesh deflection class.  */
class ICollision : public ReferenceTarget {
public:
//return what is supported for collision engine
//right now all we support is point to surface collision
//but in the future the others maybe support by us or 3rd party
//it returns the or'd flags above
	/*! \remarks This method returns the collision type supported by the
	engine.
	\return  One of the following;\n\n
	<b>POINT_COLLISION</b> for point collision, currently supported.\n\n
	<b>SPHERE_COLLISION</b> for spherical collision, currently not
	supported.\n\n
	<b>BOX_COLLISION</b> for box collision, currently not supported.\n\n
	<b>EDGE_COLLISION</b> for edge collision, currently not supported. */
	virtual int SuppportedCollisions() = 0; 

//This method is called once before the checkcollision is called for each frame
//which allows you to do some data initializations
	/*! \remarks This method will be called once before the checkcollision is
	called for each frame which allows you to do any required initialization.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	virtual void PreFrame(TimeValue t, TimeValue dt) = 0;
//This method is called at the end f each frame solve to allow 
//you to destroy any data you don't need want
	/*! \remarks This method will be called at the end of each frame solve to
	allow you to destroy and deallocate any data you no longer need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	virtual void PostFrame(TimeValue t, TimeValue dt) = 0;

//point to surface collision
//computes the time at which the particle hit the surface 
//t is the end time of the particle 
//dt is the delta of time that particle travels
//   t-dt = start of time of the particle
//pos the position of the particle in world space
//vel the velocity of the particle in world space
//at is the point in time that the collision occurs with respect to dt
//norm is bounce vector component of the final velocity
//friction is the friction vector component of the final velocity
//inheritVel is the amount of velocity inherited from the motion of the delfector
//		this is a rough apporximate
	/*! \remarks This method will be called to execute a point to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
								 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel) = 0;

//sphere to surface collision
	/*! \remarks This method will be called to execute a sphere to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (TimeValue t,Point3 pos, float radius, Point3 vel, float dt, 
								 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel) = 0;
//box to surface collision
	/*! \remarks This method will be called to execute a box to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Box3 box</b>\n\n
	The box itself.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (TimeValue t, Box3 box, Point3 vel, float dt,  
								 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel) = 0;
//edge to surface collision
	/*! \remarks This method will be called to execute an edge to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 edgeA</b>\n\n
	The first edge.\n\n
	<b>Point3 edgeB</b>\n\n
	The second edge.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (TimeValue t,Point3 edgeA,Point3 edgeB ,Point3 vel, float dt,  
								 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel) = 0;
};


enum { collision_supportedcollisions, collision_preframe,collision_postframe,
	   collision_point_to_surface,collision_sphere_to_surface ,collision_box_to_surface,
	   collision_edge_to_surface   };


/*! \sa  Class ICollision, Class CollisionPlane, Class CollisionSphere, Class CollisionVNormal, Class CollisionMesh, Class Box3,  Class Point3,  Class FPInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the operation interface to the collision detection
system.\n\n
The interface ID is defined as <b>COLLISION_FO_INTERFACE.</b> To obtain a
pointer to this interface you can use the macro
<b>GetCollisionOpsInterface(cd)</b>, which will return <b>(CollisionOps
*)(cd)-\>GetFPInterface(COLLISION_FO_INTERFACE)</b>.  */
class CollisionOps : public FPInterface
{
public:
	/*! \remarks This method returns the collision type supported by the
	engine.\n\n

	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target to check the collision type for.
	\return  One of the following;\n\n
	<b>POINT_COLLISION</b> for point collision, currently supported.\n\n
	<b>SPHERE_COLLISION</b> for spherical collision, currently not
	supported.\n\n
	<b>BOX_COLLISION</b> for box collision, currently not supported.\n\n
	<b>EDGE_COLLISION</b> for edge collision, currently not supported. */
	virtual int SuppportedCollisions(ReferenceTarget *r) = 0; 
	/*! \remarks This method will be called once before the checkcollision is
	called for each frame which allows you to do any required initialization.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	virtual void PreFrame(ReferenceTarget *r, TimeValue &t, TimeValue &dt) = 0;
	/*! \remarks This method will be called at the end of each frame solve to
	allow you to destroy and deallocate any data you no longer need.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	virtual void PostFrame(ReferenceTarget *r, TimeValue &t, TimeValue &dt) = 0;
	/*! \remarks This method will be called to execute a point to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the <b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *pos, Point3 *vel, float &dt, 
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel) = 0;

//sphere to surface collision
	/*! \remarks This method will be called to execute a sphere to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the <b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *pos, float &radius, Point3 *vel, float &dt, 
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel) = 0;
//box to surface collision FIX ME can't publish box3
	/*! \remarks This method will be called to execute a box to surface collision
	and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Box3 box</b>\n\n
	The box itself.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the <b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (ReferenceTarget *r, TimeValue &t, 
								 Point3 *boxCenter,float &w, float &h, float &d, Point3 *vel, float &dt,  
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel) = 0;

//edge to surface collision
	/*! \remarks This method will be called to execute an edge to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>ReferenceTarget *r</b>\n\n
	A pointer to the reference target.\n\n
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 edgeA</b>\n\n
	The first edge.\n\n
	<b>Point3 edgeB</b>\n\n
	The second edge.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the <b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	virtual BOOL CheckCollision (ReferenceTarget *r, TimeValue &t,Point3 *edgeA,Point3 *edgeB ,Point3 *vel, float &dt,  
								 float &at, Point3 *hitPoint, Point3 *norm, Point3 *friction, Point3 *inheritedVel) = 0;


};


// block IDs
enum { collisionplane_params, };

// geo_param param IDs
enum { collisionplane_width,
	   collisionplane_height, 
	   collisionplane_quality,
	   collisionplane_node,    
	};

#pragma warning(push)
#pragma warning(disable:4239 4100)

/*! \sa  Class ICollision,
Class CollisionOps, Class CollisionSphere, Class CollisionVNormal, Class CollisionMesh, Class Box3,  Class Point3,  Class IParamBlock2, Class INode,  Class Control\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the planar collision object with the ClassID defined as
<b>PLANAR_COLLISION_ID</b>. This class allows you to define a plane in space
and determine if a particle hit it.
\par Data Members:
private:\n\n
<b>INode *node;</b>\n\n
The associated node.\n\n
<b>IParamBlock2 *pblock;</b>\n\n
The parameter block data. You can use the following enum parameter ID's:\n\n
<b>collisionplane_width</b>\n\n
<b>collisionplane_height</b>\n\n
<b>collisionplane_quality</b>\n\n
<b>collisionplane_node</b>\n\n
<b>Interval validity;</b>\n\n
The validity interval.\n\n
<b>Matrix3 tm;</b>\n\n
The plane's TM.\n\n
<b>Matrix3 invtm;</b>\n\n
The inverse TM.\n\n
<b>Matrix3 prevInvTm;</b>\n\n
The cached previous inverse TM.\n\n
<b>int initialTime;</b>\n\n
The initial time.\n\n
<b>Tab\<Matrix3\> invTmList;</b>\n\n
The table of inverse TM's.\n\n
<b>float width, height;</b>\n\n
The width and height of the plane.\n\n
<b>int quality;</b>\n\n
The collision quality value.  */
class CollisionPlane : public ICollision
{
private:
	INode *node;
public:
	IParamBlock2 *pblock;
	Interval validity;

	/*! \remarks Constructor. */
	CollisionPlane();
	/*! \remarks Destructor. */
	~CollisionPlane();
//determines what type of collisions are supported
	/*! \remarks This method determines the type of collisions that are
	supported.
	\return  One of the following;\n\n
	<b>POINT_COLLISION</b> for point collision, currently supported.\n\n
	<b>SPHERE_COLLISION</b> for spherical collision, currently not
	supported.\n\n
	<b>BOX_COLLISION</b> for box collision, currently not supported.\n\n
	<b>EDGE_COLLISION</b> for edge collision, currently not supported.
	\par Default Implementation:
	<b>{ return POINT_COLLISION; }</b> */
	int SuppportedCollisions() 
		{ 
		return POINT_COLLISION; 
		} 

	/*! \remarks This method will be called once before the checkcollision is
	called for each frame which allows you to do any required initialization.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	void PreFrame(TimeValue t, TimeValue dt) ;
	/*! \remarks This method will be called at the end of each frame solve to
	allow you to destroy and deallocate any data you no longer need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel.
	\par Default Implementation:
	<b>{}</b> */
	void PostFrame(TimeValue t, TimeValue dt) {}

	/*! \remarks This method will be called to execute a point to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel);
//sphere to surface collision
	/*! \remarks This method will be called to execute a sphere to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, float radius, Point3 vel, float dt, 
		                 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//box to surface collision
	/*! \remarks This method will be called to execute a box to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Box3 box</b>\n\n
	The box itself.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t, Box3 box, Point3 vel, float dt,  
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}
//edge to surface collision
	/*! \remarks This method will be called to execute an edge to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 edgeA</b>\n\n
	The first edge.\n\n
	<b>Point3 edgeB</b>\n\n
	The second edge.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 edgeA,Point3 edgeB ,Point3 vel,  float dt,  
						float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//access functions to the pblock
	/*! \remarks Sets the width of the plane.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the width.\n\n
	<b>float w</b>\n\n
	The width.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionplane_width,t,w); }</b> */
	void SetWidth(TimeValue t, float w)  { if (!pblock->GetControllerByID(collisionplane_width)) pblock->SetValue(collisionplane_width,t,w); }
	/*! \remarks Sets the height of the plane.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the height.\n\n
	<b>float h</b>\n\n
	The height.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionplane_height,t,h); }</b> */
	void SetHeight(TimeValue t, float h) { if (!pblock->GetControllerByID(collisionplane_height)) pblock->SetValue(collisionplane_height,t,h); }
	/*! \remarks Sets the quality of the solve. This is the maximum number of
	iterations the solver will take to find the hit point. The lower quality
	the mire likely a particle will leak through the surface but the faster the
	solver will be.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the quality.\n\n
	<b>int q</b>\n\n
	The quality value.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionplane_quality,t,q); }</b> */
	void SetQuality(TimeValue t, int q)  { if (!pblock->GetControllerByID(collisionplane_quality)) pblock->SetValue(collisionplane_quality,t,q); }
//	void SetTM(TimeValue t, Matrix3 tm)	 { pblock->SetValue(collisionplane_tm,t,&tm); }
	/*! \remarks Sets the node which drives the TM to put the plane in world
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the node.\n\n
	<b>INode *n</b>\n\n
	The node to set.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionplane_node,t,n); node = n; }</b> */
	void SetNode(TimeValue t, INode *n)	 
	{ 
		{
			HoldSuspend hs;
			pblock->SetValue(collisionplane_node,t,n);
		}
		node = n; 
}

	/*! \remarks Sets the controller for the plane width.
	\par Parameters:
	<b>Control *c</b>\n\n
	A pointer to the controller to set.
	\par Default Implementation:
	<b>{ pblock-\>SetControllerByID(collisionplane_width,0,c); }</b> */
	void SetWidth(Control *c) { pblock->SetControllerByID(collisionplane_width,0,c,FALSE); }
	/*! \remarks Sets the controller for the plane height.
	\par Parameters:
	<b>Control *c</b>\n\n
	A pointer to the controller to set.
	\par Default Implementation:
	<b>{ pblock-\>SetControllerByID(collisionplane_height,0,c); }</b> */
	void SetHeight(Control *c) { pblock->SetControllerByID(collisionplane_height,0,c,FALSE); }
	/*! \remarks Sets the quality of the solve. This is the maximum number of
	iterations the solver will take to find the hit point. The lower quality
	the mire likely a particle will leak through the surface but the faster the
	solver will be.
	\par Parameters:
	<b>Control *c</b>\n\n
	A pointer to the controller to set.
	\par Default Implementation:
	<b>{ pblock-\>SetControllerByID(collisionplane_quality,0,c); }</b> */
	void SetQuality(Control *c) { pblock->SetControllerByID(collisionplane_quality,0,c,FALSE); }
//	void SetTM(Control *c);

	Matrix3 tm, invtm;
	Matrix3 prevInvTm;

	int initialTime;
	Tab<Matrix3> invTmList;
	float  width, height;
	int quality;



	// Methods from Animatable
	/*! \remarks Self deletion.
	\par Default Implementation:
	<b>{ delete this; }</b> */
	void DeleteThis();
	/*! \remarks This method returns the class ID.
	\par Default Implementation:
	<b>{return PLANAR_COLLISION_ID;}</b> */
	Class_ID ClassID() {return PLANAR_COLLISION_ID;}
	/*! \remarks This method returns the super class ID.
	\par Default Implementation:
	<b>{return REF_MAKER_CLASS_ID;}</b> */
	SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

	// Methods from ReferenceTarget :
	/*! \remarks This method returns the number of references.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	int NumRefs() { return 1; }
	/*! \remarks This method returns the I-th parameter block.
	\par Default Implementation:
	<b>{ return pblock; }</b> */
	RefTargetHandle GetReference(int i) { return pblock; }
	/*! \remarks This method allows you to set the I-th parameter block.
	\par Parameters:
	<b>int i</b>\n\n
	The I-th parameter block to set.\n\n
	<b>RefTargetHandle rtarg</b>\n\n
	The reference target handle to the parameter block.
	\par Default Implementation:
	<b>{pblock = (IParamBlock2*)rtarg;}</b> */
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}
public:
	/*! \remarks This method is called to have the plug-in clone itself. This
	method should copy both the data structure and all the data residing in the
	data structure of this reference target. The plug-in should clone all its
	references as well.
	\par Parameters:
	<b>RemapDir \&remap</b>\n\n
	This class is used for remapping references during a Clone. See
	Class RemapDir.
	\return  A pointer to the cloned item. */
 	RefTargetHandle Clone(RemapDir &remap);      

	/*! \remarks A plug-in which makes references must implement this method
	to receive and respond to messages broadcast by its dependents.
	\par Parameters:
	<b>Interval changeInt</b>\n\n
	This is the interval of time over which the message is active.\n\n
	<b>RefTargetHandle hTarget</b>\n\n
	This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference
	target sent the message.\n\n
	<b>PartID\& partID</b>\n\n
	This contains information specific to the message passed in. Some messages
	don't use the \ref partids at all. See \ref Reference_Messages and \ref partids for more information.\n\n
	<b>RefMessage message</b>\n\n
	The msg parameters passed into this method is the specific message which
	needs to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type RefResult. This is
	usually <b>REF_SUCCEED</b> indicating the message was processed. Sometimes,
	the return value may be <b>REF_STOP</b>. This return value is used to stop
	the message from being propagated to the dependents of the item. */
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);

};





// block IDs
enum { collisionsphere_params, };

// geo_param param IDs
enum Collision_Params { collisionsphere_radius,
	   collisionsphere_node,    //using a node right now this really needs to be a TM but it does not look like tms are hooked up yet in pb2
	   collisionsphere_scaleFactor
	};


/*! \sa  Class ICollision, Class CollisionOps, Class CollisionPlane, Class CollisionVNormal, Class CollisionMesh, Class Box3,  Class Point3,  Class IParamBlock2, Class INode,  Class Control\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the planar collision object with the ClassID defined as
<b>SPHERICAL_COLLISION_ID</b>. This class allows you to define a plane in space
and determine if a particle hit it.
\par Data Members:
private:\n\n
<b>INode *node;</b>\n\n
The associated node.\n\n
<b>IParamBlock2 *pblock;</b>\n\n
The parameter block data. You can use the following enum parameter ID's:\n\n
<b>collisionsphere_radius</b>\n\n
<b>collisionsphere_node</b>\n\n
<b>Interval validity;</b>\n\n
The validity interval.\n\n
<b>Matrix3 tm;</b>\n\n
The plane's TM.\n\n
<b>Matrix3 invtm;</b>\n\n
The inverse TM.\n\n
<b>Matrix3 prevInvTm;</b>\n\n
The cached previous inverse TM.\n\n
<b>float radius;</b>\n\n
The radius of the sphere.  */
class CollisionSphere : public ICollision
{
private:
	INode *node;
public:
	IParamBlock2 *pblock;
	Interval validity;

	/*! \remarks Constructor. */
	CollisionSphere();
	/*! \remarks Destructor. */
	~CollisionSphere();
//determines what type of collisions are supported
	/*! \remarks This method determines the type of collisions that are
	supported.
	\return  One of the following;\n\n
	<b>POINT_COLLISION</b> for point collision, currently supported.\n\n
	<b>SPHERE_COLLISION</b> for spherical collision, currently not
	supported.\n\n
	<b>BOX_COLLISION</b> for box collision, currently not supported.\n\n
	<b>EDGE_COLLISION</b> for edge collision, currently not supported.
	\par Default Implementation:
	<b>{ return POINT_COLLISION; }</b> */
	int SuppportedCollisions() 
		{ 
		return POINT_COLLISION; 
		} 

	/*! \remarks This method will be called once before the checkcollision is
	called for each frame which allows you to do any required initialization.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	void PreFrame(TimeValue t, TimeValue dt) ;
	/*! \remarks This method will be called at the end of each frame solve to
	allow you to destroy and deallocate any data you no longer need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel.
	\par Default Implementation:
	<b>{}</b> */
	void PostFrame(TimeValue t, TimeValue dt) {}

	/*! \remarks This method will be called to execute a point to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel);
//sphere to surface collision
	/*! \remarks This method will be called to execute a sphere to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, float radius, Point3 vel, float dt, 
		                 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//box to surface collision
	/*! \remarks This method will be called to execute a box to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Box3 box</b>\n\n
	The box itself.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t, Box3 box, Point3 vel, float dt,  
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}
//edge to surface collision
	/*! \remarks This method will be called to execute an edge to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 edgeA</b>\n\n
	The first edge.\n\n
	<b>Point3 edgeB</b>\n\n
	The second edge.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 edgeA,Point3 edgeB ,Point3 vel,  float dt,  
						float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//access functions to the pblock
	/*! \remarks Sets the radius of the sphere.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the radius.\n\n
	<b>float r</b>\n\n
	The radius.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionsphere_radius,t,r); }</b> */
	void SetRadius(TimeValue t, float r)  { if (!pblock->GetControllerByID(collisionsphere_radius)) pblock->SetValue(collisionsphere_radius,t,r); }
	/*! \remarks Sets the node which drives the TM to put the plane in world
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the node.\n\n
	<b>INode *n</b>\n\n
	The node to set.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionsphere_node,t,n); node = n; }</b> */
	void SetNode(TimeValue t, INode *n)	 
	{ 
		{
			HoldSuspend hs;
			pblock->SetValue(collisionsphere_node,t,n);
		}
		node = n; 
}

	/*! \remarks Sets the radius controller.
	\par Parameters:
	<b>Control *c</b>\n\n
	A pointer to the radius controller.
	\par Default Implementation:
	<b>{ pblock-\>SetControllerByID(collisionsphere_radius,0,c); }</b> */
	void SetRadius(Control *c) { pblock->SetControllerByID(collisionsphere_radius,0,c,FALSE); }

	Matrix3 tm, invtm;
	Matrix3 prevInvTm;
	Point3 Vc;



	float  radius;



	// Methods from Animatable
	/*! \remarks Self deletion.
	\par Default Implementation:
	<b>{ delete this; }</b> */
	void DeleteThis();
	/*! \remarks This method returns the class ID.
	\par Default Implementation:
	<b>{return SPHERICAL_COLLISION_ID;}</b> */
	Class_ID ClassID() {return SPHERICAL_COLLISION_ID;}
	/*! \remarks This method returns the super class ID.
	\par Default Implementation:
	<b>{return REF_MAKER_CLASS_ID;}</b> */
	SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

	// Methods from ReferenceTarget :
	/*! \remarks This method returns the number of references.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	int NumRefs() { return 1; }
	/*! \remarks This method returns the I-th parameter block.
	\par Default Implementation:
	<b>{ return pblock; }</b> */
	RefTargetHandle GetReference(int i) { return pblock; }
	/*! \remarks This method allows you to set the I-th parameter block.
	\par Parameters:
	<b>int i</b>\n\n
	The I-th parameter block to set.\n\n
	<b>RefTargetHandle rtarg</b>\n\n
	The reference target handle to the parameter block.
	\par Default Implementation:
	<b>{pblock = (IParamBlock2*)rtarg;}</b> */
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}
public:
	/*! \remarks This method is called to have the plug-in clone itself. This
	method should copy both the data structure and all the data residing in the
	data structure of this reference target. The plug-in should clone all its
	references as well.
	\par Parameters:
	<b>RemapDir \&remap</b>\n\n
	This class is used for remapping references during a Clone. See
	Class RemapDir.
	\return  A pointer to the cloned item. */
 	RefTargetHandle Clone(RemapDir &remap);      

	/*! \remarks A plug-in which makes references must implement this method
	to receive and respond to messages broadcast by its dependents.
	\par Parameters:
	<b>Interval changeInt</b>\n\n
	This is the interval of time over which the message is active.\n\n
	<b>RefTargetHandle hTarget</b>\n\n
	This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference
	target sent the message.\n\n
	<b>PartID\& partID</b>\n\n
	This contains information specific to the message passed in. Some messages
	don't use the partID at all. See \ref Reference_Messages and
	\ref partids for more information.\n\n
	<b>RefMessage message</b>\n\n
	The msg parameters passed into this method is the specific message which
	needs to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type RefResult. This is
	usually <b>REF_SUCCEED</b> indicating the message was processed. Sometimes,
	the return value may be <b>REF_STOP</b>. This return value is used to stop
	the message from being propagated to the dependents of the item. */
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message);
};



/*! \sa  Class ICollision, Class CollisionOps, Class CollisionPlane, Class CollisionSphere, Class CollisionMesh, Class Box3,  Class Point3\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents a general list of collision vertex normals.
\par Data Members:
<b>Point3 norm;</b>\n\n
The normal vector\n\n
<b>DWORD smooth;</b>\n\n
The smoothing flag.\n\n
<b>CollisionVNormal *next;</b>\n\n
A pointer to the next normal in the linked list.\n\n
<b>BOOL init;</b>\n\n
The initialization flag.  */
class CollisionVNormal: public MaxHeapOperators {
	public:
		Point3 norm;
		DWORD smooth;
		CollisionVNormal *next;
		BOOL init;

		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{smooth=0;next=NULL;init=FALSE;norm=Point3(0,0,0);}</b> */
		CollisionVNormal() {smooth=0;next=NULL;init=FALSE;norm=Point3(0,0,0);}
		/*! \remarks Constructor.
		\par Parameters:
		<b>Point3 \&n</b>\n\n
		The vector to initialize with.\n\n
		<b>DWORD s</b>\n\n
		The smoothing flag to initialize with.
		\par Default Implementation:
		<b>{next=NULL;init=TRUE;norm=n;smooth=s;}</b> */
		CollisionVNormal(Point3 &n,DWORD s) {next=NULL;init=TRUE;norm=n;smooth=s;}
		/*! \remarks Destructor.
		\par Default Implementation:
		<b>{delete next;}</b> */
		~CollisionVNormal();
		/*! \remarks Add a vector to the list.
		\par Parameters:
		<b>Point3 \&n</b>\n\n
		The vector to add.\n\n
		<b>DWORD s</b>\n\n
		The smoothing flag to add. */
		void AddNormal(Point3 &n,DWORD s);
		/*! \remarks Returns the specified normal from the list.
		\par Parameters:
		<b>DWORD s</b>\n\n
		The index of the normal in the list. */
		Point3 &GetNormal(DWORD s);
		/*! \remarks This method normalizes the vector. */
		void Normalize();
	};


// block IDs
enum { collisionmesh_params, };

// geo_param param IDs
enum { 
		collisionmesh_hit_face_index,
		collisionmesh_hit_bary,
		collisionmesh_node    //using a node right now this really needs to be a TM but it does not look like tms are hooked up yet in pb2
	};


/*! \sa  Class ICollision,
Class CollisionOps, Class CollisionPlane, Class CollisionSphere, Class CollisionVNormal, Class Box3,  Class Point3,  Class IParamBlock2, Class INode,  Class Control\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the planar collision object with the ClassID defined as
<b>MESH_COLLISION_ID</b>. This class allows you to define a plane in space and
determine if a particle hit it.
\par Data Members:
private:\n\n
<b>INode *node;</b>\n\n
The associated node.\n\n
<b>IParamBlock2 *pblock;</b>\n\n
The parameter block data. You can use the following enum parameter ID's:\n\n
<b>collisionmesh_hit_face_index</b>\n\n
<b>collisionmesh_hit_bary</b>\n\n
<b>collisionmesh_node</b>\n\n
<b>Interval validity;</b>\n\n
The validity interval.\n\n
<b>Matrix3 tm;</b>\n\n
The plane's TM.\n\n
<b>Matrix3 invtm;</b>\n\n
The inverse TM.\n\n
<b>Matrix3 tmPrev;</b>\n\n
The previous TM.\n\n
<b>Matrix3 prevInvTm;</b>\n\n
The cached previous inverse TM.\n\n
<b>float radius;</b>\n\n
The radius of the sphere.\n\n
<b>Mesh *dmesh;</b>\n\n
The mesh pointer.\n\n
<b>int nv, nf;</b>\n\n
The mesh number of vertices and number of faces.\n\n
<b>CollisionVNormal *vnorms;</b>\n\n
The collision vertex normals.\n\n
<b>Point3 *fnorms;</b>\n\n
The face normals.  */
class CollisionMesh : public ICollision
{
private:
	INode *node;
public:
	IParamBlock2 *pblock;
	Interval validity;
	DWORD outFi;
	Point3 outBary;

	/*! \remarks Constructor. */
	CollisionMesh();
	/*! \remarks Destructor. */
	~CollisionMesh();
//determines what type of collisions are supported
	/*! \remarks This method determines the type of collisions that are
	supported.
	\return  One of the following;\n\n
	<b>POINT_COLLISION</b> for point collision, currently supported.\n\n
	<b>SPHERE_COLLISION</b> for spherical collision, currently not
	supported.\n\n
	<b>BOX_COLLISION</b> for box collision, currently not supported.\n\n
	<b>EDGE_COLLISION</b> for edge collision, currently not supported.
	\par Default Implementation:
	<b>{ return POINT_COLLISION; }</b> */
	int SuppportedCollisions() 
		{ 
		return POINT_COLLISION; 
		} 

	/*! \remarks This method will be called once before the checkcollision is
	called for each frame which allows you to do any required initialization.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel. */
	void PreFrame(TimeValue t, TimeValue dt) ;
	/*! \remarks This method will be called at the end of each frame solve to
	allow you to destroy and deallocate any data you no longer need.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to initialize.\n\n
	<b>TimeValue dt</b>\n\n
	The delta of time the particle wil travel.
	\par Default Implementation:
	<b>{}</b> */
	void PostFrame(TimeValue t, TimeValue dt) {}

	/*! \remarks This method will be called to execute a point to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, Point3 vel, float dt, 
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel);

//sphere to surface collision
	/*! \remarks This method will be called to execute a sphere to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 pos</b>\n\n
	The position of the particle in world space.\n\n
	<b>float radius</b>\n\n
	The radius of the sphere.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 pos, float radius, Point3 vel, float dt, 
		                 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//box to surface collision
	/*! \remarks This method will be called to execute a box to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Box3 box</b>\n\n
	The box itself.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t, Box3 box, Point3 vel, float dt,  
						 float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}
//edge to surface collision
	/*! \remarks This method will be called to execute an edge to surface
	collision and compute the time at which the particle hit the surface.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The end time of the particle.\n\n
	<b>Point3 edgeA</b>\n\n
	The first edge.\n\n
	<b>Point3 edgeB</b>\n\n
	The second edge.\n\n
	<b>Point3 vel</b>\n\n
	The velocity of the particle in world space.\n\n
	<b>float dt</b>\n\n
	The delta of time that the particle travels (<b>t-dt</b> being the start of
	time of the particle)\n\n
	<b>float \&at</b>\n\n
	The point in time that the collision occurs with respect to the
	<b>dt</b>.\n\n
	<b>Point3 \&hitPoint</b>\n\n
	The point of collision.\n\n
	<b>Point3 \&norm</b>\n\n
	The bounce vector component of the final velocity.\n\n
	<b>Point3 \&friction</b>\n\n
	The friction vector component of the final velocity.\n\n
	<b>Point3 inheritedVel</b>\n\n
	The approximated amount of velocity inherited from the motion of the
	deflector.
	\return  TRUE if there's a collision, otherwise FALSE. */
	BOOL CheckCollision (TimeValue t,Point3 edgeA,Point3 edgeB ,Point3 vel,  float dt,  
						float &at, Point3 &hitPoint, Point3 &norm, Point3 &friction, Point3 &inheritedVel)
		{
		return FALSE;
		}

//access functions to the pblock
	/*! \remarks Sets the node which drives the TM to put the plane in world
	space.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to set the node.\n\n
	<b>INode *n</b>\n\n
	The node to set.
	\par Default Implementation:
	<b>{ pblock-\>SetValue(collisionmesh_node,t,n); node = n; }</b> */
	void SetNode(TimeValue t, INode *n)	 { 
//check for circle loop here

										   pblock->SetValue(collisionmesh_node,t,n);
										   node = n; }


	Matrix3 tm, invtm;
	Matrix3 tmPrev,invtmPrev;

	Mesh *dmesh;
	int nv,nf;
	CollisionVNormal *vnorms;
	Point3 *fnorms;

//	Mesh *dmeshPrev;
//	VNormal *vnormsPrev;
//	Point3 *fnormsPrev;

	// Methods from Animatable
	/*! \remarks Self deletion.
	\par Default Implementation:
	<b>{ delete this; }</b> */
	void DeleteThis();
	/*! \remarks This method returns the class ID.
	\par Default Implementation:
	<b>{return SPHERICAL_COLLISION_ID;}</b> */
	Class_ID ClassID() {return MESH_COLLISION_ID;}
	/*! \remarks This method returns the super class ID.
	\par Default Implementation:
	<b>{return REF_MAKER_CLASS_ID;}</b> */
	SClass_ID SuperClassID() {return REF_MAKER_CLASS_ID;}

	// Methods from ReferenceTarget :
	/*! \remarks This method returns the number of references.
	\par Default Implementation:
	<b>{ return 1; }</b> */
	int NumRefs() { return 1; }
	/*! \remarks This method returns the I-th parameter block.
	\par Default Implementation:
	<b>{ return pblock; }</b> */
	RefTargetHandle GetReference(int i) { return pblock; }
	/*! \remarks This method allows you to set the I-th parameter block.
	\par Parameters:
	<b>int i</b>\n\n
	The I-th parameter block to set.\n\n
	<b>RefTargetHandle rtarg</b>\n\n
	The reference target handle to the parameter block.
	\par Default Implementation:
	<b>{pblock = (IParamBlock2*)rtarg;}</b> */
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg) {pblock = (IParamBlock2*)rtarg;}
public:
	/*! \remarks This method is called to have the plug-in clone itself. This
	method should copy both the data structure and all the data residing in the
	data structure of this reference target. The plug-in should clone all its
	references as well.
	\par Parameters:
	<b>RemapDir \&remap</b>\n\n
	This class is used for remapping references during a Clone. See
	Class RemapDir.
	\return  A pointer to the cloned item. */
 	RefTargetHandle Clone(RemapDir &remap);      

	/*! \remarks A plug-in which makes references must implement this method
	to receive and respond to messages broadcast by its dependents.
	\par Parameters:
	<b>Interval changeInt</b>\n\n
	This is the interval of time over which the message is active.\n\n
	<b>RefTargetHandle hTarget</b>\n\n
	This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference
	target sent the message.\n\n
	<b>PartID\& partID</b>\n\n
	This contains information specific to the message passed in. Some messages
	don't use the partID at all. See \ref Reference_Messages and \ref partids for more information.\n\n
	<b>RefMessage message</b>\n\n
	The msg parameters passed into this method is the specific message which
	needs to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type RefResult. This is
	usually <b>REF_SUCCEED</b> indicating the message was processed. Sometimes,
	the return value may be <b>REF_STOP</b>. This return value is used to stop
	the message from being propagated to the dependents of the item. */
	RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,RefMessage message)
		{
		switch (message) {
			case REFMSG_CHANGE:
				if (hTarget == pblock)
					validity.SetEmpty();
				break;
			}
//note this is ref_stop because we don't want the engine updating it references
//may need a flag to turn this off or on
		return( REF_STOP);
		}
};

#pragma warning(pop)
