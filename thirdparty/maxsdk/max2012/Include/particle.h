/**********************************************************************
 *<
	FILE: particle.h

	DESCRIPTION: Particle system object

	CREATED BY: Rolf Berteig

	HISTORY: 10-18-95

 *> Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "meshlib.h"
#include "export.h"
#include "GraphicsTypes.h"

// forward declarations
class ParticleSys;

struct SphereData: public MaxHeapOperators{
 Point3 center;
 float radius,oradius,rsquare,tover4;
};

// Custom particle drawing callback
/*! \sa  Class ParticleSys, Class GraphicsWindow.\n\n
\par Description:
This class allow a plug-in particle system to provide its own custom drawing
routine. Implement the <b>DrawParticle()</b> method of this class and register
this callback with the <b>SetCustomDraw()</b> method of class
<b>ParticleSys</b>.  */
class CustomParticleDisplay: public MaxHeapOperators {
	public:
		/*! \remarks Draws the 'i-th' particle of the specified particle system.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The window into which to draw the particle.\n\n
		<b>ParticleSys \&parts</b>\n\n
		The particle system whose 'i-th' particle is to be drawn.\n\n
		<b>int i</b>\n\n
		The index of the particle to draw. */
		virtual BOOL DrawParticle(GraphicsWindow *gw,ParticleSys &parts,int i)=0;
	};

/*! \sa  Class GraphicsWindow, Marker Types, Class HitRegion.\n\n
\par Description:
This class describes a particle system. Methods are available to display, hit
test, and compute the bounding box of the particle system. Other methods
allocate and free the particles and allow custom particle drawing procedures to
be used.
\par Data Members:
<b>Tab\<Point3\> points;</b>\n\n
The location of each particle.\n\n
<b>Tab\<Point3\> vels;</b>\n\n
The velocity of each particle (optional).\n\n
<b>Tab\<TimeValue\> ages;</b>\n\n
The age of each particle (optional).\n\n
<b>float size;</b>\n\n
The world space radius of a particle.  */
class ParticleSys : public MaxHeapOperators {
	private:
		CustomParticleDisplay *draw;

		void DrawGW(GraphicsWindow *gw,DWORD flags,MarkerType type);

	public:
		Tab<Point3>	points;		// The particles themselves
		Tab<Point3> vels;		// Velocities of each particle (optional)
		Tab<TimeValue> ages;	// Age of each particle (optional)
		Tab<float> radius;
		Tab<float> tension;
		float size;				// World space radius of a particle


		// Draws the particle system into the GW
		/*! \remarks Implemented by the System.\n\n
		Draws the particle system into the GraphicsWindow.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window into which to particle system is to be drawn.\n\n
		<b>MarkerType type=POINT_MRKR</b>\n\n
		One of the following values:\n\n
		See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">Marker Types</a>. */
		DllExport void Render(GraphicsWindow *gw,MarkerType type=POINT_MRKR);
		
		// Hit tests the particle system. Returns TRUE if a particle is hit.
		/*! \remarks Implemented by the System.\n\n
		This method hit tests the particle system and returns TRUE if the
		particle system was hit.
		\par Parameters:
		<b>GraphicsWindow *gw</b>\n\n
		The graphics window to hit test in.\n\n
		<b>HitRegion *hr</b>\n\n
		Pointer to an instance of HitRegion describing the hit test region.\n\n
		<b>int abortOnHit=FALSE</b>\n\n
		If TRUE the hit testing should be aborted upon the first successful
		hit; otherwise hit testing should continue through all particles.\n\n
		<b>MarkerType type=POINT_MRKR</b>\n\n
		The type of particle marker being used. One of the following
		values:\n\n
		See <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_marker_types.html">Marker Types</a>.
		\return  TRUE if a particle is hit; otherwise FALSE. */
		DllExport BOOL HitTest(GraphicsWindow *gw, HitRegion *hr, 
			int abortOnHit=FALSE,MarkerType type=POINT_MRKR);

		// Gets bounding box
		/*! \remarks Implemented by the System.\n\n
		Returns the 3D bounding box of the particles.
		\par Parameters:
		<b>Matrix3 *tm=NULL</b>\n\n
		If not NULL, this is the optional space to compute the bounding box in.
		*/
		DllExport Box3 BoundBox(Matrix3 *tm=NULL);

		// Sets all counts to 0
		/*! \remarks Implemented by the System.\n\n
		Sets all the counts to 0. This will be the points, and if used, the
		velocities and ages. */
		DllExport void FreeAll();

		// Sets the size. Flags indicate if optional params should be allocated
		/*! \remarks Implemented by the System.\n\n
		Sets the size of the particle system. This is to at least set the
		number of points in the particle system. The flags indicate if optional
		parameters velocities and ages should be allocated as well.
		\par Parameters:
		<b>int c</b>\n\n
		The size for each allocated table.\n\n
		<b>DWORD flags</b>\n\n
		One or more of the following values:\n\n
		<b>PARTICLE_VELS</b> - Velocities should be allocated.\n\n
		<b>PARTICLE_AGES</b> - Particles ages should be allocated. */
		DllExport void SetCount(int c,DWORD flags);

		/*! \remarks Implemented by the System.\n\n
		Returns the number of points in the particle system. */
		int Count() {return points.Count();}
		/*! \remarks Implemented by the System.\n\n
		Returns the 'i-th' point of the particle system. */
		Point3& operator[](int i) {return points[i];}

		// Is particle i alive?
		/*! \remarks Implemented by the System.\n\n
		Determines if particle <b>i</b> is alive (has not expired).
		\par Parameters:
		<b>int i</b>\n\n
		The index of the particle to check.
		\return  TRUE if the 'i-th' particle is alive; otherwise FALSE. */
		BOOL Alive(int i) {return ages[i]>=0;}

		// Sets custom draw callback
		/*! \remarks Implemented by the System.\n\n
		Establishes a custom draw callback object. This allows the particles to
		be displayed in any manner desired (not just using the standard point
		markers). See Class CustomParticleDisplay.
		\par Parameters:
		<b>CustomParticleDisplay *d</b>\n\n
		The custom draw callback object.
		\par Operators:
		*/
		void SetCustomDraw(CustomParticleDisplay *d) {draw=d;}
	};

// Flags for SetCount()
#define PARTICLE_VELS	(1<<0)
#define PARTICLE_AGES	(1<<1)
#define PARTICLE_RADIUS	(1<<2)
#define PARTICLE_TENSION (1<<3)

class MetaParticle: public MaxHeapOperators {
	public:
		DllExport int CreateMetas(ParticleSys parts,Mesh *mesh,float threshold,float res,float strength,int many=1);
		DllExport int CreatePodMetas(SphereData *data,int num,Mesh *mesh,float threshold,float res,int many=1);
};
