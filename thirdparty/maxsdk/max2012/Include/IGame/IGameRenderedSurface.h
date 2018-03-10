/**********************************************************************
 *<
	FILE: IGameRenderedSurface.h

	DESCRIPTION: IGameRenderSurface interfaces for IGame.  This interface
	provide access to the lighting and color of an object at the face level.
	Any IGame Object can provide access to the data calculated here.  It uses
	the same basic approach as the Vertex Color Utility

	CREATED BY: Neil Hazzard, Discreet 

	HISTORY: created 12/11/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "..\maxheap.h"
#include <WTypes.h>
#include "IGameExport.h"

/*!\file IGameRenderedSurface.h
\brief Access to the main render routines to calculate surface data such as Lighting and color.

Lighting and color can be calculated on a per face/vertex level.  This data is the same technique that
the Apply Vertex color utility uses.  The difference here is that you are provided the color independently of the
vertex color channel.  This leaves that channel for other data and provides more options at export time.
*/


#define EVAL_MAPS		(1<<0)  //!< Flag indicates if texture maps should be applied
#define EVAL_SHADOWS	(1<<1)  //!< Flag indicates if shadows should be applied

namespace LightingModel
{
   // SR NOTE64: Merge with istdplug.h's definition? (different values!)
   //! The possible lighting models
   enum LightingModel_Game 	
   {
		SHADED,            //!< Store shaded color with lighting
		LIGHTING_ONLY,     //!< Store lighting only
		SHADED_ONLY        //!< Store shaded color without lighting
   };
}

//!class IGameRenderedSurface
/*! Access to the main render surface interface.  Methods are available to setup the system and define the lighting models
used.  Actual access to the data is provided through the IGameMesh class
*/
class IGameRenderedSurface: public MaxHeapOperators
{
public:
	
   typedef LightingModel::LightingModel_Game   LightingModel;
	//! \brief Destructor 
	 virtual ~IGameRenderedSurface() {;}

	//! Setup the database for rendering.
	virtual bool InitialiseRenderedSurfaceData() = 0;

	//! Clean it all up
	virtual void ReleaseRenderedSurfaceData() = 0 ;

	//! Define the type of model be used in the render
	virtual void SetLightingModel(LightingModel lm) = 0;

	//! Define the usage of Maps and shadows
	virtual void SetFlag(DWORD mask) = 0;

	//! Define the usage of Maps and shadows
	virtual void ClearFlag(DWORD mask) = 0;
};

//! Get the main interface pointer.
IGAMEEXPORT IGameRenderedSurface *GetIGameRenderedSurfaceInterface();

