/**********************************************************************
 *<
	FILE:			sbmtlapi.h

	DESCRIPTION:	Object Sub Material API

	CREATED BY:		Christer Janson

	HISTORY:		1-19-98

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/

// By deriving your object or modifier from this class you will support
// direct assignment of sub materials to selected faces/elements.
// For a reference implementation of this class, please refer to
// maxsdk\samples\pack1\triobjed.cpp

#pragma once
#include "maxheap.h"
#include "maxtypes.h"
// forward declarations
class ModContext;

/*! \sa  <a href="ms-its:3dsmaxsdk.chm::/mtls_materials.html">Working with
Materials</a>.
\par Description:
This class is available in release 2.5 and later only.\n\n
In 3ds Max 2.5 this is only supported for objects flowing down the geometry
pipeline.\n\n
This class is used if you wish to support the direct assignment of sub
materials to selected faces. For a reference implementation of this class,
please refer to the code in
<b>/MAXSDK/SAMPLES/MESH/EDITABLEMESH/TRIOBJED.CPP</b>.  */
class ISubMtlAPI: public MaxHeapOperators {
public:
	/*! \remarks This method returns a material ID that is currently not used by
	the object. If the current face selection share one single MtlDI that is not
	used by any other faces, you should use it. */
	virtual MtlID	GetNextAvailMtlID(ModContext* mc) = 0;

	/*! \remarks This method indicates if you are active in the modifier panel and
	have an active face selection. Return TRUE if so, otherwise FALSE. */
	virtual BOOL	HasFaceSelection(ModContext* mc) = 0;
	/*! \remarks This method sets the selected faces to the specified material	ID..
	\par Parameters:
	<b>MtlID id</b>\n\n
	The material id to set for the selected faces.\n\n
	<b>BOOL bResetUnsel = FALSE</b>\n\n
	If TRUE, then you should set the remaining unselected face material IDs to 0.	*/
	virtual void	SetSelFaceMtlID(ModContext* mc, MtlID id, BOOL bResetUnsel = FALSE) = 0;
	/*! \remarks This method returns the material ID of the selected face(s). If
	multiple faces are selected they should all have the same MtlID -- otherwise
	you should return -1. If faces other than the selected share the same material
	ID, then return -1. */
	virtual int		GetSelFaceUniqueMtlID(ModContext* mc) = 0;
	/*! \remarks This method returns the material ID of the selected face(s). If
	multiple faces are selected they should all have the same MtlID, otherwise
	return -1. */
	virtual int		GetSelFaceAnyMtlID(ModContext* mc) = 0;
	/*! \remarks This method returns the highest MtlID on the object. */
	virtual	int		GetMaxMtlID(ModContext* mc) = 0;
};


