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

#pragma once

#include "export.h"
#include "maxheap.h"
#include "UVWMapTypes.h"
#include "matrix3.h"

// Translates map type into 3d location -> uvw coord mapper:
/*! \sa  Class Mesh, Class Matrix3, Class Point3.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
Prior to release 3.0, developers could implement <b>Object::ApplyUVWMap()</b>
in their objects, but didn't have access to the algorithm 3ds Max uses
internally to turn the mapping types (MAP_BOX, MAP_PLANE, etc) into an actual
vertex-to-mapping-coordinate function. This class now makes this available.\n\n
The constructors for the class initialize the data members with information
about the mapping desired. The main method, <b>MapPoint()</b>, maps a point in
object space into the UVW map defined by this mapper.\n\n
Note: <b>typedef Point3 UVVert;</b>
\par Data Members:
<b>int type;</b>\n\n
The mapping type. One of the following values:\n\n
<b>MAP_PLANAR</b>\n\n
<b>MAP_CYLINDRICAL</b>\n\n
<b>MAP_SPHERICAL</b>\n\n
<b>MAP_BALL</b>\n\n
<b>MAP_BOX</b>\n\n
<b>MAP_FACE</b>\n\n
<b>int cap;</b>\n\n
This is used with <b>MAP_CYLINDRICAL</b>. If nonzero, then any face normal that
is pointing more vertically than horizontally will be mapped using planar
coordinates.\n\n
<b>float utile;</b>\n\n
Number of tiles in the U direction.\n\n
<b>float vtile;</b>\n\n
Number of tiles in the V direction.\n\n
<b>float wtile;</b>\n\n
Number of tiles in the W direction.\n\n
<b>int uflip;</b>\n\n
If nonzero the U values are mirrored.\n\n
<b>int vflip</b>\n\n
If nonzero the V values are mirrored.\n\n
<b>int wflip;</b>\n\n
If nonzero the W values are mirrored.\n\n
<b>Matrix3 tm;</b>\n\n
This defines the mapping space. As each point is mapped, it is multiplied by
this matrix, and then it is mapped.  */
class UVWMapper: public MaxHeapOperators {
public:
	int     type, cap;
	float   utile, vtile, wtile;
	int     uflip, vflip, wflip;
	Matrix3 tm;

	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> type = MAP_BOX;</b>\n\n
	<b> utile = 1.0f;</b>\n\n
	<b> vtile = 1.0f;</b>\n\n
	<b> wtile = 1.0f;</b>\n\n
	<b> uflip = 0;</b>\n\n
	<b> vflip = 0;</b>\n\n
	<b> wflip = 0;</b>\n\n
	<b> cap = 0;</b>\n\n
	<b> tm.IdentityMatrix();</b> */
	DllExport UVWMapper();
	/*! \remarks Constructor. The data members are initialized to the values
	passed. */
	DllExport UVWMapper(int type, const Matrix3 &tm, int cap=FALSE,
		float utile=1.0f, float vtile=1.0f, float wtile=1.0f,
		int uflip=FALSE, int vflip=FALSE, int wflip=FALSE);
	/*! \remarks Constructor. The data members are initialized from the
	UVWMapper passed. */
	DllExport UVWMapper(UVWMapper& m);

	/*! \remarks This method maps a point in object space into the UVW map
	defined by this mapper. This gives the UVW coordinates for the specified
	point according to this mapper's mapping scheme.
	\par Parameters:
	<b>Point3 p</b>\n\n
	The location of a vertex, i.e. the point being mapped. This point should
	NOT be transformed by the UVWMapper's tm, as this happens internally.\n\n
	<b>const Point3 \& norm</b>\n\n
	The direction of the surface normal at p. This information is only required
	for types MAP_BOX or MAP_CYLINDRICAL. See the method <b>NormalMatters()</b>
	below.\n\n
	<b>int *nan=NULL</b>\n\n
	If non-NULL, this points to an int which should be set to FALSE if this
	mapping is good for all faces using this vertex, or TRUE if different faces
	should have different mapping coordinates. This is generally set to TRUE
	more often than absolutely necessary to make sure nothing is missed.
	\return  The mapped point. */
	DllExport UVVert MapPoint(Point3 p, const Point3 & norm, int *nan=NULL);
	/*! \remarks Applies the UVWMap's tile and flip parameters to the given
	UVVert, and returns the result..
	\par Parameters:
	<b>UVVert uvw</b>\n\n
	The input UVVert.
	\return  The modified UVVert. */
	DllExport UVVert TileFlip (UVVert uvw);
	/*! \remarks This method indicates which direction the given vector
	"chiefly points", after vector transformation by the UVWMapper's transform.
	\par Parameters:
	<b>const Point3 \& n</b>\n\n
	The input vector whose main axis is determined.
	\return  One of the following values:\n\n
	<b>0</b>: tm.VectorTransform(n) points mainly in the +x direction.\n\n
	<b>1</b>: tm.VectorTransform(n) points mainly in the +y direction.\n\n
	<b>2</b>: tm.VectorTransform(n) points mainly in the +z direction.\n\n
	<b>3</b>: tm.VectorTransform(n) points mainly in the -x direction.\n\n
	<b>4</b>: tm.VectorTransform(n) points mainly in the -y direction.\n\n
	<b>5</b>: tm.VectorTransform(n) points mainly in the -z direction. */
	DllExport int MainAxis (const Point3 & n);
	/*! \remarks This method lets you know whether the current mapping type
	uses the normal information. If FALSE, it doesn't matter what value you
	pass as a normal to MapPoint. If TRUE, the MainAxis of the normal is used
	to determine the mapping. */
	bool NormalMatters () { return ((type==MAP_BOX || type==MAP_ACAD_BOX)||((type==MAP_CYLINDRICAL || type==MAP_ACAD_CYLINDRICAL )&&cap)) ? TRUE:FALSE; }
};

