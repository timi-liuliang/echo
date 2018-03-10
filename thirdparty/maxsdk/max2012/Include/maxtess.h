/**********************************************************************
 *<
	FILE: maxtess.h

	DESCRIPTION: Tessellation Approximation class

	CREATED BY: Charles Thaeler

	HISTORY: created 12 Dec 1996
			 Updated 12-10-98 Peter Watje to support hidden interior edges

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"

// forward declarations
class ILoad;
class ISave;

enum TessType
{
	TESS_SET,		// This is the old MAX form for Bezier Patches
	TESS_ISO,		// This is obsolete and should not be used.
	TESS_PARAM,
	TESS_SPATIAL,
	TESS_CURVE,
	TESS_LDA,
	TESS_REGULAR 
};

enum ViewConfig
{
	ISO_ONLY,
	ISO_AND_MESH,
	MESH_ONLY
};

enum TessSubdivStyle
{
	SUBDIV_TREE,
	SUBDIV_GRID,
	SUBDIV_DELAUNAY
};

/*! \sa  Class NURBSSet.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class describes the properties of a tesselation approximation to the
mathematical surface.\n\n
All methods of this class are implemented by the system.
\par Data Members:
<b>TessType type;</b>\n\n
These are the types of tesselation (one of which is obsolete). One of the
following values:\n\n
<b>TESS_SET</b>\n\n
This is the old form of tesselation for Bezier Patches. This is also the
default for these patches. For instance, if you create a Quad Patch and apply
an Edit Patch modifier, then exit sub-object mode, you'll see a panel in the
rollup for 'Tesselation'. The top choice is 'Fixed (original)'. This is the
same type of tesselation done in 3ds Max 1.x.\n\n
<b>TESS_PARAM</b>\n\n
Specifies parametric tesselation. This provides for a fixed number of <b>u</b>
by <b>v</b> tesselations. There are <b>u</b> times <b>v</b> quadrilaterals and
each one is split up into two triangles.\n\n
<b>TESS_SPATIAL</b>\n\n
Specifies spatial tesselation. This uses <b>edge</b> as its parameter. This
specifies that the size of the tesselation will be the <b>edge</b> length (see
below). In view dependent tesselation <b>edge</b> is specified in pixels.\n\n
<b>TESS_CURVE</b>\n\n
Specifies view dependent tesselation. This uses the <b>ang</b> and <b>dist</b>
data members described below.\n\n
<b>TESS_LDA</b>\n\n
This option is available in release 3.0 and later only.\n\n
Specifies a method which combines the spatial (edge-length) method and the
curvature (distance and angle) methods. This uses the <b>ang</b>, <b>dist</b>
and <b>edge</b> data members below.\n\n
<b>TESS_REGULAR</b>\n\n
This option is available in release 3.0 and later only.\n\n
Generates a fixed, regular tessellation across the surface. There are no
additional parameters.\n\n
<b>TESS_ISO</b>\n\n
Obsolete -- Do Not Use.\n\n
<b>ViewConfig vpt_cfg;</b>\n\n
This determines what is displayed in the interactive renderer. These correspond
to the controls in the user interface (under Surface/Approximation/Viewports).
This is not available for bezier patches. One of the following values:\n\n
<b>ISO_ONLY</b>\n\n
Only Iso lines. Iso(parametric) lines are similar to contour lines. The lines
show where the NURBS surface has a constant U value or V value or both. Iso
line representations can be less crowded and easier to visualize than wire mesh
representations..\n\n
<b>ISO_AND_MESH</b>\n\n
Iso lines and the mesh. When chosen, wireframe viewports display iso line
representations of the surface, and shaded viewports display the shaded
surface.\n\n
<b>MESH_ONLY</b>\n\n
Just the mesh. When chosen, wireframe viewports display the surface as a wire
mesh, and shaded viewports display the shaded surface. In wireframe viewports,
this option lets you see the curve approximation used for viewports.\n\n
<b>TessSubdivStyle subdiv;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
The type of subdivision. One of the following values:\n\n
<b>SUBDIV_TREE</b>\n\n
Subdivides the surface using a binary tree.\n\n
<b>SUBDIV_GRID</b>\n\n
Subdivides the surface using a regular grid.\n\n
<b>SUBDIV_DELAUNAY</b>\n\n
Subdivides the surface using nearly equilateral triangles.\n\n
<b>BOOL view;</b>\n\n
Specifies if this is view dependent tesselation. If TRUE this will tesselate
less finely the farther away from the camera the object is. If FALSE the
tesselation does not change based on distance from the camera.\n\n
<b>int u;</b>\n\n
This is used for parametric tesselation. This is the number of tesselations in
u. This is the number of sub-divisions for a knot span for the surface.\n\n
<b>int v;</b>\n\n
This is used for parametric tesselation. This is the number of tesselations in
v.\n\n
<b>int u_iso;</b>\n\n
This is used with the ISO line display. This is the number of additional
<b>interior</b> iso lines in u (there are always lines along the outter
edges).\n\n
<b>int v_iso;</b>\n\n
This is used with the ISO line display. This is the number of additional
<b>interior</b> iso lines in v (there are always lines along the outter
edges).\n\n
<b>float ang;</b>\n\n
This is used in curvature dependent tesselation (<b>TESS_CURVE</b>). If 0.0 is
specified this is ignored. If specified this ensure that no two adjacent face
normals exceed this angle between them. This value is specified in radians.\n\n
<b>float dist;</b>\n\n
This is used in curvature dependent tesselation (<b>TESS_CURVE</b>). If 0.0 is
specified this is ignored. This specifies a distance that cannot be exceeded
between a vertex on the mesh and the mathematical surface. This is defined as a
percentage of the diagonal of the bounding box of the individual surface in
object space. For instance if this was set to 1.0, the allowable error in
generating a tesselation would be 1% of the bounding box diagonal distance of
the surface. This would be 1/100 (1 %) of the diagonal distance of the bounding
box. In this way if an object is scaled the tesselation remains the same.
Additionally, if you have an object with a big surface and a little surface,
the smaller surface will get tesselated more finely because its own bounding
box is used. This prevents the smaller surface from just becoming a single
triangle for example.\n\n
<b>float edge;</b>\n\n
This is the length of an edge to use in spatial (<b>TESS_SPATIAL</b>)
tesselation. In view dependent tesselation this is specified in pixels. If not
in view dependent tesselation this is a percentage of the bounding box diagonal
length.\n\n
<b>int minSub;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
For Grid or Tree subdivisions, this limit controls the number of recursive
decompositions that are performed during tessellation. This is the minimum
number of recursions.\n\n
<b>int maxSub;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
For Grid or Tree subdivisions, this limit controls the number of recursive
decompositions that are performed during tessellation. This is the maximum
number of recursions.\n\n
<b>int maxTris;</b>\n\n
This data member is available in release 3.0 and later only.\n\n
For Delaunay subdivision, this specifies the maximum mumber of triangles into
which the surface will be divided.  */
class TessApprox: public MaxHeapOperators {
public:
	TessType type;
	ViewConfig vpt_cfg;
	TessSubdivStyle subdiv;
	BOOL view;
	float merge;
	int u, v;
	int u_iso, v_iso;
	float ang, dist, edge;
	int minSub, maxSub, maxTris;

//watje 12-10-98
	BOOL showInteriorFaces;

	/*! \remarks Constructor. The data members are initialized as follows:\n\n
	<b> type = TESS_SET;</b>\n\n
	<b> u = v = 2;</b>\n\n
	<b> u_iso = 2;</b>\n\n
	<b> v_iso = 3;</b>\n\n
	<b> view = FALSE;</b>\n\n
	<b> ang = 20.0f;</b>\n\n
	<b> dist = 10.0f;</b>\n\n
	<b> edge = 10.0f;</b>\n\n
	<b> vpt_cfg = ISO_AND_MESH;</b>\n\n
	<b> merge = 0.0f;</b>\n\n
	<b> minSub = 0;</b>\n\n
	<b> maxSub = 5;</b>\n\n
	<b> maxTris = 20000;</b>\n\n
	<b> subdiv = SUBDIV_TREE;</b>\n\n
	<b> showInteriorFaces = FALSE;</b> */
	UtilExport TessApprox();
	/*! \remarks	Constructor. The data members are initialized to the values passed. */
	UtilExport TessApprox(TessType type, float distance, float edge, float angle,
                          TessSubdivStyle subdivStyle, int minSub, int maxSub,
                          float m = 0.0f);
	/*! \remarks Constructor. The data members are initialized from the object
	passed.
	\par Operators:
	*/
	UtilExport TessApprox(const TessApprox &tess);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const TessApprox \&tess</b>\n\n
	The object to assign. */
	UtilExport TessApprox & operator=(const TessApprox& tess);
	/*! \remarks Equality operator. Returns nonzero if they are equal;
	otherwise zero.
	\par Parameters:
	<b>const TessApprox \&tess</b>\n\n
	The object to compare. */
	UtilExport int operator==(const TessApprox &tess) const;


	UtilExport IOResult Load(ILoad* iload);
	UtilExport IOResult Save(ISave* isave);
};

