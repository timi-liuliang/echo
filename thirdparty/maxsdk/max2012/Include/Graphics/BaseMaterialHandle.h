//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "./SmartHandle.h"
#include "../acolor.h"

namespace MaxSDK { namespace Graphics {

/** \deprecated Deprecated in 3ds Max 2012. No replacement.
This is our base graphics material class. A material describes how 
to render the surface of an object. This class, and all its derived classes, are 
used for viewport display and Quicksilver. 
The difference between BaseMaterialHandle and ISimpleMaterial is that 
BaseMaterialHandle is used by RenderItemHandle::SetCustomMaterial() to describes the surface and
ISimpleMaterial is used for Mtl and Texmap to display.
*/
class MAX_DEPRECATED BaseMaterialHandle : public SmartHandle
{
public:
	GraphicsDriverAPI BaseMaterialHandle();
	GraphicsDriverAPI BaseMaterialHandle(const BaseMaterialHandle& from);
	GraphicsDriverAPI BaseMaterialHandle& operator = (const BaseMaterialHandle& from);
	GraphicsDriverAPI virtual ~BaseMaterialHandle();

	/** Returns the material required streams to tell how to setup mesh data.
	When a BaseMaterialHandle is used by RenderItemHandle::SetCustomMaterial(), 
	it also needs to know about the material requirement streams to setup corresponding 
	mesh data. It could be used by GenerateRenderItems::GenerateRenderItems() for RenderItemHandle.
	\return the material required streams.
	*/
	GraphicsDriverAPI virtual const MaterialRequiredStreams* GetRequiredStreams() const;
};

} } // end namespace
