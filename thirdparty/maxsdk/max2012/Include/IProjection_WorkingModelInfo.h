/*==============================================================================

  file:     IProjection_WorkingModelInfo.h

  author:   Daniel Levesque

  created:  19 May 2004

  description:

      Definition of an interface that gives access to basic shade context information
	  on the working model of a bake projection.

	  This interface is to be used along with the bake projection feature, which
	  allows using high-res objects when baking maps for low-res objects.

  modified:	


(c) 2004 Autodesk
==============================================================================*/

#pragma once

#include "BaseInterface.h"
#include "imtl.h"

// forward declarations
class RenderInstance;
class Point3;


//==============================================================================
// class IProjection_WorkingModelInfo
/*! \sa Class ShadeContext
	\par Description:
	This interface provides access to some shade context information for the
	working model.  It is used when performing a projection mapped Render To Texture,
	to support shading operations on the reference models which additionally 
	require information about the working model.\n\n
	This interface is provided by the renderer to Texture Bake elements, 
	when performing a Render To Texture (RTT) in projection mode. In this mode, 
	pixels are shaded as follows. First, the renderer locates a point on the working model. 
	A ray is cast to locate a corresponding point on one of the reference models. 
	Material shading proceeds on the reference model instead of the working model. 
	The material is responsible for invoking each texture bake element and render element, 
	as part of its shading operation. However, the texture bake elements might need information 
	about the working model, and the shade context only contains information for the reference model.
	The renderer solves this problem by attaching an IProjection_WorkingModelInfo interface 
	to the ShadeContext that it passes to the material.\n\n
	When a bake projection is done, all the information in the shade context
	is overriden to represent the intersection point on the projected object.
	If a bake render element still needs access to shade context information
	from the original object (the working model), then this interface may be used.\n\n
	This interface provides a render instance of the working model and information about 
	the working model point being shaded. It should be queried from the shade context 
	through ShadeContext::GetInterface(). A NULL pointer will be returned if no projection was done.

	\par Examples:
	For an example of usage, please refer to the Normal Map and Height Map texture bake elements, 
	in the SDK samples maxsdk\samples\render\renderelements\stdBakeElem.cpp.
*/
//==============================================================================
class IProjection_WorkingModelInfo : public BaseInterface {

public:

	//! Query this interface on a shade context
	static IProjection_WorkingModelInfo* GetInterface(ShadeContext& shadeContext);

	//! Returns the render instance of the working model
	virtual RenderInstance* GetRenderInstance() = 0;
	//! Returns the face number of that hit face
	virtual int GetFaceNumber() = 0;
	//! Returns the original interpolated normal
	virtual const Point3& GetOrigNormal() = 0;
	//! Returns the barycentric coordinates of the hit point
    virtual void GetBarycentricCoords(Point3& bary) = 0;    
	virtual void GetBarycentricCoords(float& b0, float& b1, float& b2) = 0;
	//! Returns whether the backface was hit
	virtual BOOL GetBackFace() = 0;
	//! Returns the point that was hit, in camera space
	virtual const Point3& GetPoint() = 0;
	//! Get the view direction, in camera space (equivalent to SContext::V())
	virtual const Point3& GetViewDir() = 0;
	//! Get the original view direction, in camera space (equivalent to SContext::OrigView())
	virtual const Point3& GetOriginalViewDir() = 0;
	//! Get the material number of the hit face
	virtual int GetMaterialNumber() = 0;
	//! Get the bump basis vectors of the hit face (equivalent to SContext::BumpBasisVectors())
	virtual void BumpBasisVectors(Point3 dP[2], int axis, int channel) = 0;
	//! Get the map basis vectors (aks bump basis vectors) interpolated at the current position
	virtual void TangentBasisVectors(Point3 dP[2], int mapChannel) = 0;

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

#define IPROJECTION_WORKINGMODELINFO_INTERFACE_ID Interface_ID(0x79907259, 0xbf05948)

inline IProjection_WorkingModelInfo* IProjection_WorkingModelInfo::GetInterface(ShadeContext& shadeContext) {

	return static_cast<IProjection_WorkingModelInfo*>(shadeContext.GetInterface(IPROJECTION_WORKINGMODELINFO_INTERFACE_ID));
}

inline Interface_ID IProjection_WorkingModelInfo::GetID() {

	return IPROJECTION_WORKINGMODELINFO_INTERFACE_ID;
}

