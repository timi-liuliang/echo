//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Values for specifying the desired type of render to a renderer.
// AUTHOR: Attila Szabo - created Aug.23.2005
//***************************************************************************/

#pragma once

#include "buildver.h"

//! \brief Used to specify the desired type of render when opening a renderer 
//! using Interface::OpenCurRenderer.
/*! The following 3 types are not passed into plugin renderers:
	RENDTYPE_REGION_SEL, RENDTYPE_CROP_SEL and RENDTYPE_BLOWUP_SEL.
	Their purpose is for passing to the function Interface::OpenCurRenderer, 
	which converts them into RENDTYPE_REGION, RENDTYPE_REGIONCROP and RENDTYPE_BLOWUP, respectively.
	The latter 3 are passed into renderers with the region dimensions specified
	by the system according to the bounding boxes of the selected objects.
	These are related to values in RenderUIType enum, but their values do not 
	necessarily match. RendType and RenderUIType are not directly interchangeable.

	\sa Interface::OpenCurRenderer, Interface7::SetRegionRect, Interface7::SetBlowupRect, RenderUIType
*/
enum RendType
{ 
	RENDTYPE_NORMAL, //!< the entire view
	RENDTYPE_REGION, //!< a portion of the entire view specified by the region rectangle.
	RENDTYPE_BLOWUP, //!< a portion of the entire view, specified by the blowup rectangle, enlarged to the dimensions of the output image
	RENDTYPE_SELECT, //!< only selected objects in the entire view
	RENDTYPE_REGIONCROP, //!< a portion of the entire view, specified by the region rectangle, cropped to a portion of the output image
	RENDTYPE_REGION_SEL, //!< a region render using the bounding rectangle of the selection
	RENDTYPE_CROP_SEL,	//!< a crop render using the bounding rectangle of the selection

	RENDTYPE_BAKE_SEL,	//!< bake textures on selected objects
	RENDTYPE_BAKE_ALL,	//!< bake textures on all objects
	RENDTYPE_BAKE_SEL_CROP,	//!< bake textures on selected objects, crop render

	RENDTYPE_BLOWUP_SEL //!< a blowup render using the bounding rectangle of the selection
};

//! \brief Used to change the current render type in the main user interface.
/*! These values specify the render type selected in the main user interface,
	using Interface7::GetRenderType and Interface7::SetRenderType.
	These are related to values in RendType enum, but their values do not 
	necessarily match. RendType and RenderUIType are not directly interchangeable.
	These values may also not match the order in which their corresponding values
	appear in the user interface. 

	\sa Interface7::GetRenderType, Interface7::SetRenderType, RendType
*/
enum RenderUIType {
	RENDER_VIEW = 0,	//!< the entire view
#ifdef USE_RENDER_REGION_SIMPLIFIED
	RENDER_REGION,		//!< a portion of the entire view specified by the region rectangle.
	RENDER_CROP,		//!< a portion of the entire view, specified by the region rectangle, cropped to a portion of the output image
#else
	RENDER_SELECTED,	//!< only selected objects in the entire view
	RENDER_REGION,		//!< a portion of the entire view specified by the region rectangle.
	RENDER_CROP,		//!< a portion of the entire view, specified by the region rectangle, cropped to a portion of the output image
	RENDER_BLOWUP,		//!< a portion of the entire view, specified by the blowup rectangle, enlarged to the dimensions of the output image
	RENDER_BLOWUP_SELECTED,	//!< a blowup render using the bounding rectangle of the selection
	RENDER_REGION_SELECTED, //!< a region render using the bounding rectangle of the selection
	RENDER_CROP_SELECTED	//!< a crop render using the bounding rectangle of the selection
#endif
};

