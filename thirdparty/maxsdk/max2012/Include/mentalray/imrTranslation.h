//*****************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
/*==============================================================================

  file:     imrTranslation.h

  author:   Daniel Levesque

  created:  16 Feb 2006

  description:

    Defition of the central mental ray translation interface

  modified:	

==============================================================================*/
#pragma once

#include "..\baseinterface.h"
#include "..\maxtypes.h"

// Forward declarations
class INode;
class Box3;
class Interval;

//==============================================================================
// class imrTranslation
//
//! \brief Access interface for the mental ray translator.
/*!	This interface is the access point for the mental ray translator. Any
	functionality that needs to be exposed to various mental ray API classes
	is exposed through this.
*/
class imrTranslation : public InterfaceServer {

public:

	//! \brief Calculates and returns the bounding box of every object to be rendered,
	//! in the object space of a given node.
	/*!	Implements a caching mechanism to avoid re-calculating the same bounding box
		multiple times during the same render operation. The calculated bounding box
		only includes objects to be rendered.
		\param[in] t - The time at which the bounding box is to be calculated.
		\param[in] nodeForSpace - The bounding box is calculated in the object space
		of this node. Pass NULL in order to calculate the bounding box in world space.
		\param[out] sceneBox - Return value for the calculated bounding box.
		\param[inout] valid - Modified validity interval; intersected with the validity
		interval of the scene.
	*/
	virtual void CalculateSceneBoundingBox(TimeValue t, INode* nodeForSpace, Box3& sceneBox, Interval& valid) = 0;
};

