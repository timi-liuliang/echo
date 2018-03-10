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
// FILE:        gizmoimp.h
// DESCRIPTION: General atmoshperic gizmo objects
// AUTHOR:      Rolf Berteig
// HISTORY:     4-15-96
//              11-13-96 Moved into core
//**************************************************************************/
#pragma once
#include "coreexp.h"
#include "gizmo.h"

#define SPHEREGIZMO_CLASSID	Class_ID(0x3bc31904, 0x67d74ec7)
#define CYLGIZMO_CLASSID	Class_ID(0x3bc31904, 0x67d74ec8)
#define BOXGIZMO_CLASSID	Class_ID(0x3bc31904, 0x67d74ec9)

#pragma warning(push)
#pragma warning(disable:4239)

/*! \sa  Class GizmoObject, Class CylGizmoObject, Class BoxGizmoObject.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is a class developer can use to provide a spherical gizmo helper object
for their plug-ins. It provides implementations of all the required methods.
The following #defines are used to access the parameters from the <b>pblock</b>
pointer of the base class <b>GizmoObject</b>.\n\n
<b>#define PB_GIZMO_RADIUS 0</b>\n\n
<b>#define PB_GIZMO_HEMI 1</b>\n\n
<b>#define PB_GIZMO_SEED 2</b>\n\n
The ClassID for this class is defined as: <b>SPHEREGIZMO_CLASSID</b> */
class SphereGizmoObject : public GizmoObject {
	public:		
		CoreExport SphereGizmoObject();
		CoreExport ~SphereGizmoObject();

		// From BaseObject
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport MCHAR *GetObjectName();
		CoreExport void InitNodeName(MSTR& s);

		// Animatable methods
		CoreExport void GetClassName(MSTR& s);
		CoreExport void DeleteThis();
		Class_ID ClassID() {return SPHEREGIZMO_CLASSID;}
		
		// From ref
		CoreExport RefTargetHandle Clone(RemapDir& remap);		

		// From GizmoObject		
		Interval ObjectValidity(TimeValue t); // mjm - 1.27.99	
		CoreExport void InvalidateUI();
		CoreExport ParamDimension *GetParameterDim(int pbIndex);
		CoreExport MSTR GetParameterName(int pbIndex);
		CoreExport void DrawGizmo(TimeValue t,GraphicsWindow *gw);		
		CoreExport void GetBoundBox(Matrix3 &mat,TimeValue t,Box3 &box);
	};

#define PB_GIZMO_RADIUS	0
#define PB_GIZMO_HEMI	1
#define PB_GIZMO_SEED	2


/*! \sa  Class GizmoObject, Class BoxGizmoObject, Class SphereGizmoObject.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is a class developer can use to provide a cylindrical gizmo helper object
for their plug-ins. It provides implementations of all the required methods.
The following #defines are used to access the parameters from the <b>pblock</b>
pointer of the base class <b>GizmoObject</b>.\n\n
<b>#define PB_CYLGIZMO_RADIUS 0</b>\n\n
<b>#define PB_CYLGIZMO_HEIGHT 1</b>\n\n
<b>#define PB_CYLGIZMO_SEED 2</b>\n\n
The ClassID for this class is defined as: <b>CYLGIZMO_CLASSID</b> */
class CylGizmoObject : public GizmoObject {
	public:		
		CoreExport CylGizmoObject();
		CoreExport ~CylGizmoObject();

		// From BaseObject
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport MCHAR *GetObjectName();
		CoreExport void InitNodeName(MSTR& s);

		// Animatable methods
		CoreExport void GetClassName(MSTR& s);
		CoreExport void DeleteThis();
		Class_ID ClassID() {return CYLGIZMO_CLASSID;}
		
		// From ref
		CoreExport RefTargetHandle Clone(RemapDir& remap);		

		// From GizmoObject		
		Interval ObjectValidity(TimeValue t); // mjm - 1.27.99	
		CoreExport void InvalidateUI();
		CoreExport ParamDimension *GetParameterDim(int pbIndex);
		CoreExport MSTR GetParameterName(int pbIndex);
		CoreExport void DrawGizmo(TimeValue t,GraphicsWindow *gw);		
		CoreExport void GetBoundBox(Matrix3 &mat,TimeValue t,Box3 &box);
	};

#define PB_CYLGIZMO_RADIUS	0
#define PB_CYLGIZMO_HEIGHT	1
#define PB_CYLGIZMO_SEED	2

/*! \sa  Class GizmoObject, Class CylGizmoObject, Class SphereGizmoObject.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is a class developer can use to provide a box gizmo helper object for
their plug-ins. It provides implementations of all the required methods. The
following #defines are used to access the parameters from the <b>pblock</b>
pointer of the base class <b>GizmoObject</b>.\n\n
<b>#define PB_BOXGIZMO_LENGTH 0</b>\n\n
<b>#define PB_BOXGIZMO_WIDTH 1</b>\n\n
<b>#define PB_BOXGIZMO_HEIGHT 2</b>\n\n
<b>#define PB_BOXGIZMO_SEED 3</b>\n\n
The ClassID for this class is defined as: <b>BOXGIZMO_CLASSID</b> */
class BoxGizmoObject : public GizmoObject {
	public:
		CoreExport BoxGizmoObject();
		CoreExport ~BoxGizmoObject();

		// From BaseObject
		CoreExport CreateMouseCallBack* GetCreateMouseCallBack();
		CoreExport void BeginEditParams( IObjParam *ip, ULONG flags,Animatable *prev);
		CoreExport void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
		CoreExport MCHAR *GetObjectName();
		CoreExport void InitNodeName(MSTR& s);

		// Animatable methods
		CoreExport void GetClassName(MSTR& s);
		CoreExport void DeleteThis();
		Class_ID ClassID() {return BOXGIZMO_CLASSID;}
		
		// From ref
		CoreExport RefTargetHandle Clone(RemapDir& remap);		

		// From GizmoObject		
		Interval ObjectValidity(TimeValue t); // mjm - 1.27.99	
		CoreExport void InvalidateUI();
		CoreExport ParamDimension *GetParameterDim(int pbIndex);
		CoreExport MSTR GetParameterName(int pbIndex);
		CoreExport void DrawGizmo(TimeValue t,GraphicsWindow *gw);		
		CoreExport void GetBoundBox(Matrix3 &mat,TimeValue t,Box3 &box);
	};

#pragma warning(pop)

#define PB_BOXGIZMO_LENGTH	0
#define PB_BOXGIZMO_WIDTH	1
#define PB_BOXGIZMO_HEIGHT	2
#define PB_BOXGIZMO_SEED	3


