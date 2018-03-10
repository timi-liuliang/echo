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
// FILE: SpringSys.h
// DESCRIPTION: Public header files for the Spring System created by Adam Felt.
//                  To use this interface you will need to link to "SpringSys.lib"
//
//                  This spring system is a multi-point spring-based dynamic system.
//                  In order to your the sytem you must derive from SpringSysClient and implement
//                  Tab<Matrix3> SpringSysClient::GetForceMatrices(TimeValue t)
//                  You are responsible for gathering the spring constraint forces at any given time.
//                  You initialize the 
//
// AUTHOR:      Adam Felt
// HISTORY: 
//**************************************************************************/
#pragma once

#include "maxheap.h"
#include "baseinterface.h"
#include "tab.h"
#include "point3.h"
#include "matrix3.h"

#ifndef SpringSysExport
#	ifdef BLD_SPRING_SYS
#		define SpringSysExport __declspec( dllexport )
#	else
#		define SpringSysExport __declspec( dllimport )
#	endif
#endif

//A Controlled Particle Class
//Used to constrain a point to an object
/*! \sa  Class Point3 , Class SpringSys\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the constraint point object as it is used in the spring
system. This controlled particle class is used to constrain a point to an
object.\n\n

\par Data Members:
<b>int index</b>\n\n
This bone index is used to identify the bone.\n\n
This usually refers to a reference item or paramblock index.\n\n
<b>Point3 vel, pos;</b>\n\n
The control node stored velocity and position.  */
class SSConstraintPoint: public MaxHeapOperators  
{						
	public:
		int index;		//this bone index is used to identify the bone.  
						//Usually refers to a reference index, or paramblock index...
		Point3 pos;		//The control nodes stored position
		Point3 vel;		//The control nodes stored velocity

		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{ index = -1; pos = Point3(0.0f, 0.0f, 0.0f);</b>\n\n
		<b>vel = Point3(0.0f, 0.0f, 0.0f); }</b> */
		SSConstraintPoint() {index = -1; pos = Point3(0.0f, 0.0f, 0.0f); vel = Point3(0.0f, 0.0f, 0.0f);}
		/*! \remarks Constructor.
		\par Parameters:
		<b>int id</b>\n\n
		The index to initialize with.
		\par Default Implementation:
		<b>{index = id; vel = Point3(0,0,0);}</b> */
		SSConstraintPoint(int id) {index = id; pos = Point3(0.0f, 0.0f, 0.0f); vel = Point3(0,0,0);}
		/*! \remarks Destructor.
		\par Default Implementation:
		<b>{ }</b> */
		~SSConstraintPoint() {}
		/*! \remarks Assignment operator. */
		SSConstraintPoint& operator=(const SSConstraintPoint& from)
		{
			index = from.index;
			pos = from.pos;
			vel = from.vel;
			
			return *this;
		}

		/*! \remarks This method allows you to copy the data from the
		specified <b>SSConstraintPoint</b>.
		\par Parameters:
		<b>const SSConstraintPoint from</b>\n\n
		The object to copy from. */
		SSConstraintPoint Copy(const SSConstraintPoint from)
		{
			index = from.index;
			pos = from.pos;
			vel = from.vel;
			
			return *this;
		}
		
		/*! \remarks This method returns the value of the index. */
		int GetIndex() { return index; }
		/*! \remarks This method allows you to set the index. */
		void SetIndex(int id) { index = id; }
		/*! \remarks This method returns the position data. */
		Point3 GetPos() { return pos; }
		/*! \remarks This method allows you to set the position data.
		\par Parameters:
		<b>Point3 p</b>\n\n
		The position to set. */
		void SetPos(Point3 p) { pos = p; }
		/*! \remarks This method returns the velocity data. */
		Point3 GetVel() { return vel; }
		/*! \remarks This method allows you to set the velocity data.
		\par Parameters:
		<b>Point3 v</b>\n\n
		The velocity data to set. */
		void SetVel(Point3 v) { vel = v; }
};

//Class used to store a particles state
/*! \sa  Class SSConstraintPoint,  Class SpringSys\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the particle cache to store the state of particle data as
it is used in the spring system.\n\n

\par Data Members:
<b>Point3 vel, pos;</b>\n\n
The control node stored velocity and position.\n\n
<b>Tab\<SSConstraintPoint\> bone;</b>\n\n
The table of spring system constraint points.  */
class SSParticleCache: public MaxHeapOperators
{
	public:
		Point3 pos;
		Point3 vel;
		Tab<SSConstraintPoint> bone;

		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{pos = Point3(0,0,0); vel = Point3(0,0,0);}</b> */
		SSParticleCache() {pos = Point3(0,0,0); vel = Point3(0,0,0); bone.Init(); }
};

//A Spring Class
//Stores the parameters for binding an object to any force with a controllable spring
/*! \sa  Class BaseInterfaceServer,  Class SSConstraintPoint,  Class SpringSys\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the particle cache to store the state of particle data as
it is used in the spring system.\n\n

\par Data Members:
private:\n\n
<b>float tension;</b>\n\n
The spring tension value.\n\n
<b>float dampening;</b>\n\n
The spring dampening value.\n\n
<b>Point3 length;</b>\n\n
The spring length.\n\n
<b>SSConstraintPoint bone;</b>\n\n
The spring system constraint point.  */
class SSSpring : public BaseInterfaceServer
{
	private:
		float tension;     
		float dampening;
		Point3 length;		//rest length
		SSConstraintPoint bone;       
		//AFParticle *p2;   //for use later to apply the reactive force

	public:

		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{</b>\n\n
		<b> bone = NULL;</b>\n\n
		<b> length = Point3(0.0f, 0.0f, 0.0f);</b>\n\n
		<b> tension = 1.0f;</b>\n\n
		<b> dampening = 0.5f;</b>\n\n
		<b>}</b> */
		SSSpring() 
		{
			bone = NULL;
			length = Point3(0.0f, 0.0f, 0.0f);
			tension = 1.0f;
			dampening = 0.5f;
		}

		/*! \remarks Constructor.\n\n
		This allows you to initialize the spring.
		\par Parameters:
		<b>SSConstraintPoint *b</b>\n\n
		The constraint point to set.\n\n
		<b>Point3 l</b>\n\n
		The spring length to set.\n\n
		<b>float t=2.0f</b>\n\n
		The tension to set.\n\n
		<b>float d=1.0f</b>\n\n
		The dampening to set.
		\par Default Implementation:
		<b>{</b>\n\n
		<b> bone = *b;</b>\n\n
		<b> length = l;</b>\n\n
		<b> tension = t;</b>\n\n
		<b> dampening = d;</b>\n\n
		<b>}</b> */
		SSSpring(SSConstraintPoint *b, Point3 l, float t=2.0f, float d=1.0f)
		{
			bone = *b;
			length = l;
			tension = t;
			dampening = d;
		}

		/*! \remarks Assignment operator. */
		SSSpring& operator=(const SSSpring& from)
		{
			tension = from.tension;
			dampening = from.dampening;
			length = from.length;
			bone = from.bone;

			return *this;
		}

		/*! \remarks This method allows you to copy the data from the
		specified spring object.
		\par Parameters:
		<b>const SSSpring from</b>\n\n
		The spring to copy from. */
		SSSpring Copy(const SSSpring from)
		{
			tension = from.tension;
			dampening = from.dampening;
			length = from.length;
			bone = from.bone;

			return *this;
		}

		/*! \remarks This method returns the tension value. */
		float GetTension() {return tension;}
		/*! \remarks This method allows you to set the tension value.
		\par Parameters:
		<b>float t</b>\n\n
		The tension to set. */
		void SetTension(float t) {tension = t;}
		/*! \remarks This method returns the dampening value. */
		float GetDampening() {return dampening;}
		/*! \remarks This method allows you to set the dampening value.
		\par Parameters:
		<b>float d</b>\n\n
		The dampening value to set. */
		void SetDampening(float d) {dampening = d;}
		/*! \remarks This method returns the length of the spring. */
		Point3 GetLength() {return length;}
		/*! \remarks This method allows you to set the spring length.
		\par Parameters:
		<b>Point3 len</b>\n\n
		The spring length to set. */
		void SetLength(Point3 len) {length = len;}

		/*! \remarks This method returns the point constraint data. */
		SSConstraintPoint* GetPointConstraint() { return &bone;}
		/*! \remarks This method allows you to set the point constraint data.
		\par Parameters:
		<b>SSConstraintPoint b</b>\n\n
		The constraint point to set. */
		void SetPointConstraint(SSConstraintPoint b) { bone = b; }
		/*! \remarks This method allos you to set the point constraint data.
		\par Parameters:
		<b>int id</b>\n\n
		The index.\n\n
		<b>Point3 pos</b>\n\n
		The position data.\n\n
		<b>Point3 vel</b>\n\n
		The velocity data. */
		void SetPointConstraint(int id, Point3 pos, Point3 vel) 
		{
			bone.SetIndex(id);
			bone.SetPos(pos);
			bone.SetVel(vel);
		}

};

/*! \sa  Class SpringSys, Class SSSpring, Class SSConstraintPoint\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the spring system particle.\n\n

\par Data Members:
private:\n\n
<b>float mass;</b>\n\n
The spring particle mass.\n\n
<b>float drag;</b>\n\n
The spring particle drag.\n\n
<b>Point3 pos, vel, force;</b>\n\n
The spring particle position, velocity, and force.\n\n
<b>Tab\<SSSpring\> springs;</b>\n\n
The table of spring system springs.  */
class SSParticle: public MaxHeapOperators
{
	friend class SpringSys;

	private:
		float mass;		//the particle's mass
		float drag;		//the particle's drag coefficient
		Point3 pos;		//particle position
		Point3 vel;		//particle velocity
		Point3 force;   //Force accumulator
		Tab<SSSpring> springs;

	public:
		/*! \remarks Constructor.
		\par Default Implementation:
		<b>{</b>\n\n
		<b> mass = 300.0f;</b>\n\n
		<b> drag = 1.0f;</b>\n\n
		<b> pos = vel = force = Point3(0.0f,0.0f,0.0f);</b>\n\n
		<b> springs.ZeroCount();</b>\n\n
		<b>}</b> */
		SSParticle() 
		{
			mass = 300.0f; 
			drag = 1.0f; 
			pos = vel = force = Point3(0.0f,0.0f,0.0f); 
			springs.ZeroCount();
		}
		/*! \remarks Destructor.
		\par Default Implementation:
		<b>{ }</b> */
		~SSParticle(){}

		/*! \remarks Assignment operator. */
		SSParticle& operator=(const SSParticle& from)
		{
			mass = from.mass;
			drag = from.drag;
			pos = from.pos;
			vel = from.vel;
			force = from.force;
			springs.ZeroCount();
			springs.Shrink();
			for (int i=0;i<from.springs.Count();i++)
			{
				SSSpring spring = from.springs[i];
				springs.Append(1, &spring);
			}
			return *this;
		}

		/*! \remarks This method allows you to copy the data from the
		specified <b>SSParticle</b>.
		\par Parameters:
		<b>const SSParticle from</b>\n\n
		The spring system particle to copy from. */
		SSParticle Copy(const SSParticle from)
		{
			SSSpring spring;
			mass = from.mass;
			drag = from.drag;
			pos = from.pos;
			vel = from.vel;
			force = from.force;
			springs.ZeroCount();
			for (int i=0;i<from.springs.Count();i++)
			{
				spring.Copy(from.springs[i]);
				springs.Append(1, &spring);
			}
			return *this;
		}

		/*! \remarks This method returns the spring particle mass. */
		float GetMass() {return mass;}
		/*! \remarks This method allows you to set the spring particle mass.
		\par Parameters:
		<b>float m</b>\n\n
		The mass to set. */
		void SetMass(float m) {mass = m;}
		/*! \remarks This method returns the spring particle drag. */
		float GetDrag() {return drag;}
		/*! \remarks This method allows you to set the spring particle drag.
		\par Parameters:
		<b>float d</b>\n\n
		The drag value to set. */
		void SetDrag(float d) {drag = d;}
		/*! \remarks This method returns the spring particle position. */
		Point3 GetPos() {return pos;}
		/*! \remarks This method allows you to set the spring particle
		position.
		\par Parameters:
		<b>Point3 p</b>\n\n
		The position to set. */
		void SetPos(Point3 p) {pos = p;}
		/*! \remarks This method returns the spring particle velocity. */
		Point3 GetVel() {return vel;}
		/*! \remarks This method allows you to set the spring particle
		velocity.
		\par Parameters:
		<b>Point3 v</b>\n\n
		The velocity to set. */
		void SetVel(Point3 v) {vel = v;}
		/*! \remarks This method returns the spring particle force. */
		Point3 GetForce() {return force;}
		/*! \remarks This method allows you to set the spring particle force.
		\par Parameters:
		<b>Point3 f</b>\n\n
		The force to set. */
		void SetForce(Point3 f) {force = f;}
		/*! \remarks This method returns a pointer to the table of springs. */
		Tab<SSSpring>* GetSprings() { return &springs;}
		/*! \remarks This method returns a pointer to the I-th spring in the
		table.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the spring to return. */
		SSSpring* GetSpring(int i) {if (i>=0 && i<springs.Count()) return &(springs[i]);
									else return NULL; }
		/*! \remarks This method allows you to set the specified spring at the
		specified I-th index in the table.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the spring to set.\n\n
		<b>SSSpring spring</b>\n\n
		The spring to set. */
		void SetSpring(int i, SSSpring spring) {if (i>=0 && i<springs.Count()) 
													springs[i] = spring; }
		/*! \remarks This method allows you to set the entire spring table.
		\par Parameters:
		<b>Tab\<SSSpring\> sTab</b>\n\n
		The table of springs to set. */
		void SetSprings(Tab<SSSpring> sTab) 
		{ 
			//springs = NULL;
			springs.ZeroCount(); 
			for (int i = 0;i<sTab.Count();i++)
			{
				springs.Append(1, &(sTab[i]));
			} 
		}
		
 		/*! \remarks This method allows you to add a spring to the table.
 		Note that if the spring already exists it will be made stronger.
 		\par Parameters:
 		<b>SSConstraintPoint *bone</b>\n\n
 		A pointer to the spring system constraint point to set.\n\n
 		<b>Point3 length</b>\n\n
 		The length of the spring.\n\n
 		<b>tension=2.0f</b>\n\n
 		The tension of the spring.\n\n
 		<b>float dampening = 1.0f</b>\n\n
 		The dampening value of the spring.
 		\return  TRUE if the spring was added, FALSE if the spring was made
 		stronger. */
 		SpringSysExport BOOL AddSpring(SSConstraintPoint *bone, Point3 length, float tension=2.0f, float dampening = 1.0f);            

		/*! \remarks This method allows you to delete a spring.
		\par Parameters:
		<b>int index</b>\n\n
		The index in the table of the spring to delete. */
		void DeleteSpring(int index) 
		{		
			if ( index == 0 ) return;
			for (int i=0;i<springs.Count();i++)
			{
				if ( (springs[i].GetPointConstraint()->GetIndex()) == index)
					springs.Delete(i--, 1);
				else if (springs[i].GetPointConstraint()->GetIndex() > index)
					springs[i].GetPointConstraint()->SetIndex(springs[i].GetPointConstraint()->GetIndex()-1);
			}
		}

};

/*! \sa  Class SpringSys\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class describes a spring system client as used by the SpringSys class.
 */
class SpringSysClient: public MaxHeapOperators 
{
	public:
		/*! \remarks This method returns a table of matrices which represent
		the forces acting inside the spring system.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to get the force matrices. */
		virtual Tab<Matrix3>	GetForceMatrices(TimeValue t)=0;
		/*! \remarks This method returns the dynamic forces acting inside the
		spring system.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to get the forces.\n\n
		<b>Point3 pos</b>\n\n
		The position force.\n\n
		<b>Point3 vel</b>\n\n
		The velocity force.
		\par Default Implementation:
		<b>{ return Point3(0,0,0); }</b> */
		virtual Point3			GetDynamicsForces(TimeValue t, Point3 pos, Point3 vel)
		{
			UNUSED_PARAM(t);
			UNUSED_PARAM(pos);
			UNUSED_PARAM(vel);
			return Point3(0,0,0);
		}
};


/*! \sa  Class BaseInterfaceServer,  Class SSParticle,  Class SSParticleCache,  Class SpringSysClient ,  Class IJiggle\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is the main spring system class. This spring system is a multi-point
spring-based dynamic system. In order to your the system you must derive from
<b>SpringSysClient</b> and implement the
<b>SpringSysClient::GetForceMatrices()</b> method You are responsible for
gathering the spring constraint forces at any given time.\n\n
The spring system uses its own position and velocity to determine its motion,
as well as allowing you to add multiple spring constraints to any other objects
in the scene. Multiple constraints act like multiple links.\n\n
The solution is calculated using a start time and a step size. Solutions are
cached once for the last tick calculated, and once per frame. Performance
should be the same one each frame as you step forward, and significantly faster
after the first pass if nothing changes. Cached values will be used if going
backwards in time.\n\n

\par Data Members:
private:\n\n
<b>float referenceTime;</b>\n\n
The stored reference time.\n\n
<b>float lastTime;</b>\n\n
The stored last time.\n\n
<b>bool isValid;</b>\n\n
This flag indicates whether the spring system is valid.\n\n
<b>SSParticleCache frameCache;</b>\n\n
The spring system particle cache.\n\n
<b>Tab\<Point3\> pos_cache;</b>\n\n
The cached table of positions.\n\n
<b>Tab\<Point3\> vel_cache;</b>\n\n
The cached table of velocities.\n\n
<b>Tab\<Point3\> initPosTab;</b>\n\n
The table of initial positions.\n\n
<b>Tab\<SSParticle\> parts;</b>\n\n
The table of spring system particles.\n\n
<b>SpringSysClient* client;</b>\n\n
The spring system client object.  */
class SpringSys : public BaseInterfaceServer
{
	private:
		float referenceTime;
		float lastTime;
		bool isValid;
		
		SSParticleCache frameCache;
		Tab<Point3> pos_cache;
		Tab<Point3> vel_cache;

		Tab<Point3> initPosTab;
		Tab<SSParticle> parts;		//particles

		SpringSysClient* client;

	public:
		
		/*! \remarks Constructor. */
		SpringSys() 
		{
			client = NULL; 
			referenceTime = lastTime = 0.0f; 
			SetParticleCount(1);
			isValid = false; 
		}

		/*! \remarks Constructor.\n\n
		Initialize the class with the specified data.
		\par Parameters:
		<b>SpringSysClient* c</b>\n\n
		A pointer to the spring system client class.\n\n
		<b>int count</b>\n\n
		The number of spring system particles. */
		SpringSys(SpringSysClient* c, int count) 
		{ 
			client = c; 
			referenceTime = lastTime = 0.0f; 
			SetParticleCount(count);
			isValid = false; 
		}

		/*! \remarks Destructor. */
		~SpringSys() {}
		/*! \remarks Assignment operator. */
		SpringSysExport SpringSys& operator=(const SpringSys& from);

		/*! \remarks This method allows you to copy the data from the
		specified spring system.
		\par Parameters:
		<b>const SpringSys* from</b>\n\n
		The spring system to copy the data from. */
		SpringSysExport SpringSys Copy(const SpringSys* from);

		/*! \remarks This method allows you to set the reference time.
		\par Parameters:
		<b>float t</b>\n\n
		The reference time to set. */
		void SetReferenceTime (float t) { referenceTime = t; }
		/*! \remarks This method returns the reference time. */
		float GetReferenceTime () { return referenceTime; }
		/*! \remarks This method returns a pointer to the table of spring
		system particles. */
		Tab<SSParticle>* GetParticles() { return &parts;}
		/*! \remarks This method returns a pointer to the I-th spring system
		particle.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the spring system particle in the table. */
		SSParticle* GetParticle(int i) { if (i >=0 && i< parts.Count())	return &(parts[i]);
											else return NULL; }
		/*! \remarks This method allows you to set the number of particles in
		the spring system.
		\par Parameters:
		<b>int count</b>\n\n
		The number of particles to set. */
		SpringSysExport void SetParticleCount(int count);
		/*! \remarks This method allows you to set the initial position for
		the specified spring system particle.
		\par Parameters:
		<b>Point3 p</b>\n\n
		The initial position to set.\n\n
		<b>int partIndex</b>\n\n
		The index of the particle in the table for which to set the initial
		position. */
		SpringSysExport void SetInitialPosition (Point3 p, int partIndex);
		/*! \remarks This method allows you to set the initial velocity for
		the specified spring system particle.
		\par Parameters:
		<b>Point3 p</b>\n\n
		The initial velocity to set.\n\n
		<b>int partIndex</b>\n\n
		The index of the particle in the table for which to set the initial
		velocity. */
		SpringSysExport void SetInitialVelocity (Point3 p, int partIndex);
		/*! \remarks This method allows you to set the initial bone states for
		the spring system.
		\par Parameters:
		<b>Tab\<Matrix3\> boneTMs</b>\n\n
		The table of bone transformation matrices to set. */
		SpringSysExport void SetInitialBoneStates(Tab<Matrix3> boneTMs);
		/*! \remarks This method will invalidate the spring system and issue
		the re-computation of the spring system state. */
		SpringSysExport void Invalidate ();
		/*! \remarks This method allows you to solve the spring system
		dynamics.
		\par Parameters:
		<b>int time</b>\n\n
		The time at which to solve\n\n
		<b>float TimeDelta</b>\n\n
		The time difference. */
		SpringSysExport void Solve (int time, float TimeDelta);
		/*! \remarks This method returns the position of the specified spring
		system particle.
		\par Parameters:
		<b>Point3\& p</b>\n\n
		The position which is returned\n\n
		<b>int index</b>\n\n
		The index of the spring system particle in the table. <br>  protected:
		*/
		SpringSysExport void GetPosition (Point3& p, int index);

		SpringSysExport IOResult Load(ILoad *iload);
		SpringSysExport IOResult Save(ISave *isave);

	protected:
		/*! \remarks This method allows you to get the spring system last
		time.
		\par Default Implementation:
		<b>{ return lastTime; }</b> */
		float GetTime() { return lastTime; }
		/*! \remarks This method allows you to set the spring system last
		time.
		\par Parameters:
		<b>float t</b>\n\n
		The time to set.
		\par Default Implementation:
		<b>{ lastTime = t; }</b> */
		void SetTime(float t) { lastTime = t; }

		//force functions
		/*! \remarks This method will clear the forces acting upon the
		specified spring system particle.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void Clear_Forces(int index);
		/*! \remarks This method will compute the forces acting upon the
		specified spring system particle..
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to compute the forces.\n\n
		<b>int index</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void Compute_Forces(TimeValue t, int index);
		/*! \remarks This method will apply the drag forces onto the specified
		spring system particle.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void ApplyDrag(int index);
		/*! \remarks This method will apply the unary forces onto the
		specified spring system particle.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to apply the unary forces.\n\n
		<b>int index</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void ApplyUnaryForces(TimeValue t, int index); 
		/*! \remarks This method will compute the controlled particle force.
		\par Parameters:
		<b>Matrix3 tm</b>\n\n
		The transformation matrix.\n\n
		<b>int vertIndex</b>\n\n
		The vertex index for which to compute the controlled particle
		force.\n\n
		<b>int springIndex</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void ComputeControlledParticleForce(Matrix3 tm, int vertIndex, int springIndex);
		/*! \remarks This method will apply the spring to the specified spring
		system particle.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to apply the unary forces.\n\n
		<b>int index</b>\n\n
		The index into the table of spring system particles. */
		SpringSysExport void ApplySpring(TimeValue t, int index);

		//Solver functions
		/*! \remarks This method will update the particle state.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which to apply the unary forces.\n\n
		<b>Tab\<Matrix3\> tmArray</b>\n\n
		The table of transformation matrices.\n\n
		<b>int pIndex</b>\n\n
		The index into the table of spring system particles.\n\n
		<b>TimeValue Delta</b>\n\n
		The time difference for which to update. */
		SpringSysExport void UpdateParticleState(TimeValue t, Tab<Matrix3> tmArray, int pIndex, TimeValue Delta);
		/*! \remarks This method will compute the derivative forces for the
		specified spring system particle.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the table of spring system particles.\n\n
		<b>Point3 \&pos</b>\n\n
		The computed position.\n\n
		<b>Point3 \&vel</b>\n\n
		The computed velocity. */
		SpringSysExport void ComputeDerivative(int index, Point3 &pos, Point3 &vel);
		/*! \remarks This method retrieves the state of the specified spring
		system particle.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the table of spring system particles.\n\n
		<b>Point3 \&pos</b>\n\n
		The current position.\n\n
		<b>Point3 \&vel</b>\n\n
		The current velocity. */
		SpringSysExport void GetParticleState(int index, Point3 &pos, Point3 &vel);
		/*! \remarks This method allows you to set the particle state of the
		specified spring system particle.
		\par Parameters:
		<b>int index</b>\n\n
		The index into the table of spring system particles.\n\n
		<b>Point3 pos</b>\n\n
		The position to set.\n\n
		<b>Point3 vel</b>\n\n
		The velocity to set. */
		SpringSysExport void SetParticleState(int index, Point3 pos, Point3 vel); 
		/*! \remarks This method allows you to scale the vectors by the
		specified difference.
		\par Parameters:
		<b>Point3 \&pos</b>\n\n
		The scaled position.\n\n
		<b>Point3 \&vel</b>\n\n
		The scaled velocity.\n\n
		<b>float delta</b>\n\n
		The time difference to scale by. */
		SpringSysExport void ScaleVectors(Point3 &pos, Point3 &vel, float delta);
		/*! \remarks This method allows you to add vector forces to the spring
		system.
		\par Parameters:
		<b>Point3 pos1, vel1</b>\n\n
		The position and velocity to add.\n\n
		<b>Point3 \&pos, \&vel</b>\n\n
		The position and velocity. */
		SpringSysExport void AddVectors(Point3 pos1, Point3 vel1, Point3 &pos, Point3 &vel);
};

