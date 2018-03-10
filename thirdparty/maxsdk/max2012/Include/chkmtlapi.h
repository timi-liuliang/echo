/**********************************************************************
 *<
	FILE:			ChkMtlAPI.h

	DESCRIPTION:	Enhances material handling for particles API

	CREATED BY:		Eric Peterson

	HISTORY:		3-8-99

 *>	Copyright (c) 2000 Discreet, All Rights Reserved.
 *********************************************************************/

// This header defines interface methods used to support indirect material
// referencing and enhanced face/material associations.  Generally, these
// methods will need to be implemented by the plugin using them and cannot
// be called from a standard library, since the information required is
// intimately associated with the geometry of the 

#pragma once
#include "maxheap.h"
#include <WTypes.h>
// forward declarations
class Mtl;


/*! \sa  Class Mtl, Class RenderInstance.\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class provides interface methods used to support indirect material
referencing and enhanced face / material associations. Generally, these methods
will need to be implemented by the plug-in using them and cannot be called from
a standard library, since the information required is intimately associated
with the geometry of the object.\n\n
All methods of this class are virtual.  */
#pragma warning(push)
#pragma warning(disable:4100)
class IChkMtlAPI : public MaxHeapOperators
{
	public:
		// SupportsParticleIDbyFace returns TRUE if the object can associate
		// a particle number with a face number, and FALSE by default.
		/*! \remarks Returns TRUE if the object can associate a particle
		number with a face number, and FALSE if not.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL SupportsParticleIDbyFace() { return FALSE; }

		// GetParticleFromFace returns the particle to which the face identified
		// by faceID belongs.
		/*! \remarks Returns the particle to which the face identified by
		faceID belongs.
		\par Parameters:
		<b>int faceID</b>\n\n
		The ID of the face.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int  GetParticleFromFace(int faceID) { return 0; }

		// SupportsIndirMtlRefs returns TRUE if the object can return a material
		// pointer given a face being rendered, and FALSE if the object will be
		// associated for that render pass with only the object applied to the 
		// node,
		/*! \remarks Returns TRUE if the object can return a material pointer
		given a face being rendered, and FALSE if the object will be associated
		for that render pass with only the material applied to the node.
		\par Default Implementation:
		<b>{ return FALSE; }</b> */
		virtual BOOL SupportsIndirMtlRefs() { return FALSE; }

		// If SupportsIndirMtlRefs returns TRUE, then the following methods are meaningful.

		// NumberOfMtlsUsed returns the number of different materials used on the object
		// This number is used in enumerating the different materials via GetNthMtl and getNthMaxMtlID.
		/*! \remarks Returns the number of different materials used on the
		object. This number is used in enumerating the different materials via
		the methods <b>GetNthMtl()</b> and <b>GetNthMaxMtlID()</b> below.
		\par Default Implementation:
		<b>{ return 0; }</b>\n\n
		the following methods are meaningful. */
		virtual int NumberOfMtlsUsed() { return 0; }

		// Returns the different materials used on the object.
		/*! \remarks If the method <b>SupportsIndirMtlRefs()</b> above returns
		TRUE then this method returns the different materials used on the
		object.
		\par Parameters:
		<b>int n</b>\n\n
		The zero based index of the material used on the object.
		\return   
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		virtual Mtl* GetNthMtl(int n) { return NULL; }

		// Returns the maximum material ID number used with each of the materials on the object
		/*! \remarks If the method <b>SupportsIndirMtlRefs()</b> above returns
		TRUE then this method returns the maximum material ID number used with
		the specified material on the object
		\par Parameters:
		<b>int n</b>\n\n
		The zero based index of the material.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual int  GetNthMaxMtlID(int n) { return 0; }

		// Get MaterialFromFace returns a pointer to the material associated
		// with the face identified by faceID.
		/*! \remarks If the method <b>SupportsIndirMtlRefs()</b> above returns
		TRUE then this method returns a pointer to the material associated with
		the face identified by faceID.
		\par Parameters:
		<b>int faceID</b>\n\n
		The ID of the face to check.
		\par Default Implementation:
		<b>{ return NULL; }</b> */
		virtual Mtl* GetMaterialFromFace(int faceID) { return NULL; }
};
#pragma warning(pop)


