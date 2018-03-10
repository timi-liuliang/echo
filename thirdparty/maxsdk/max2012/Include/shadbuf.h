/**********************************************************************
 *<
	FILE: shadbuf.h : Shadow buffer renderer.

	DESCRIPTION:

	CREATED BY: Dan Silva

	HISTORY:

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include <WTypes.h>
#include "maxheap.h"
#include "matrix3.h"

// forward declarations
class RendContext;
class RenderGlobalContext;
class ObjLightDesc;

/*! \sa  Class ShadowGenerator, Class RendContext, Class RenderGlobalContext, Class ObjLightDesc, Class Matrix3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is used to generate a Shadow Buffer which may be used to determine
if a point is in shadow or not. The 3ds Max shadow maps use this object
internally, for example.\n\n
There is a global function that creates one of these <b>ShadBufRenderer</b>
objects. With one of these developers can call its <b>Render()</b> method to
generate (render) a Shadow Buffer.\n\n
The rendered shadow buffer stores a Z distance at every point in the buffer.
This can then be used to determine if something is in shadow. To check a
certain point you simply see if the Z value is behind the one in the buffer.
That is, a shadow buffer tells one, from the point of view of a light, how far
it is to the first object for each pixel in the buffer. If the Z point of the
thing being shadowed is farther than (behind) the corresponding Z value in the
buffer then the thing is in shadow. If it's closer than it is not in
shadow.\n\n
The main <b>Render()</b> method is typically called from the <b>Update()</b>
method of class <b>ShadowGenerator</b> which is called on every frame to create
a new shadow buffer.\n\n
To use this class you basically do the following:\n\n
Allocate an array of floating point values, one float for each point in the
shadow buffer:\n\n
<b> buffer = new float[shadsize*shadsize];</b>\n\n
Then create a default Shadow Buffer Renderer using the global function
provided:\n\n
<b> ShadBufRenderer *sbr = NewDefaultShadBufRenderer();</b>\n\n
Then you setup all the parameters for the view, etc prior to calling the
<b>Render()</b> method to render the buffer. (These parameters are passed in to
the <b>ShadowGenerator::Update()</b> method).\n\n
<b> int nRendered = sbr-\>Render(rc, RGC, buffer, parallel, shadsize,
param, aspect, clipDist, ltDesc, worldToLight);</b>\n\n
You can check the return value to determine if any objects were intersected by
the shadow volume. If none were, the shadow buffer can be freed.
\code
if (nRendered==0)
{
	delete [] buffer;
	sbr->DeleteThis();
	buffer = NULL;
	return 1;
}
\endcode 
All methods of this class are implemented by the system. */
class ShadBufRenderer: public MaxHeapOperators {
	public:
		/*! \remarks Compute a shadow Z buffer for the current scene from the
		viewpoint of the light. NOTE: The computed shadow buffer has positive Z values
		as you go away from the light, which is the reverse of the 3ds Max coordinate	system.
		\par Parameters:
		<b>RendContext \&rc</b>\n\n
		The RendContext which is used for the progress bar API.\n\n
		<b>RenderGlobalContext *RGC</b>\n\n
		Points to the RenerGlobalContext which is used to retireve information about
		the global rendering enviornment (to get an instance list).\n\n
		<b>float *buf</b>\n\n
		This is the buffer to render to. This is a pre-allocated array of floats
		(<b>shadsize*shadsize</b>).\n\n
		<b>BOOL parallel</b>\n\n
		The projection type. TRUE if parallel projection; FALSE if perspective
		projection.\n\n
		<b>int shadsize</b>\n\n
		The size of the buffer (shadsize by shadsize pixels).\n\n
		<b>float param</b>\n\n
		The view parameter. For a perspective this is the:field-of-view (in radians).
		For a parallel view this is the width in world coordinates.\n\n
		<b>float aspect</b>\n\n
		This is the aspect ratio of the buffer projection.\n\n
		<b>float clipDist</b>\n\n
		The clipping distance. This tells the shadow buffer renderer to not consider
		objects farther than this distance from light.\n\n
		<b>ObjLightDesc *ltDesc</b>\n\n
		This is the descriptor for light that was passed in to
		CreateShadowGenerator().\n\n
		<b>Matrix3 worldToLight</b>\n\n
		The world to light transformation matrix for the light.
		\return  Returns the number of objects that the shadow volume intersected. If
		this value is 0, the shadow buffer can be freed to save memory. */
		virtual int Render(
			RendContext &rc, 
			RenderGlobalContext *RGC, 
			float *buf,         // preallocated (shadsize*shadsize)  floats 
			BOOL parallel,		// parallel or perspective projection
			int shadsize,      // size of the buffer (shadsize by shadsize pixels)
			float param,   	   // persp:field-of-view (radians) -- parallel : width in world coords
			float aspect,      // aspect ration of the buffer projection 
			float clipDist,    // don't consider objects farther than this from light
			ObjLightDesc *ltDesc, // descriptor for light that was passed in to CreateShadowGenerator
			Matrix3 worldToLight  // world to light transfor for light
			)=0;
		/*! \remarks After a render, this method returns the farthest Z in the
		shadow buffer. */
		virtual	float Furthest()=0; // after render, this gives the farthest Z in the buffer
		/*! \remarks After a render, this method returns the closest Z in the
		shadow buffer. */
		virtual float Closest()=0;  // after render, this gives the closest Z in the buffer.
		/*! \remarks Deletes this <b>ShadowBufRenderer</b> object. */
		virtual void DeleteThis()=0;
	};

/*! \remarks This global function creates and returns a pointer to a new
default shadow buffer renderer.  */
CoreExport ShadBufRenderer *NewDefaultShadBufRenderer();

