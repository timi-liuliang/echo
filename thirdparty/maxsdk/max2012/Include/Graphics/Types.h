//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include <crtdefs.h>

namespace MaxSDK { namespace Graphics {

	/** This is the unique id type of our internal graphics objects. 
	*/
	typedef unsigned __int64 Identifier;

	/** Invalid id value.
	*/
	const Identifier NIL_ID = (Identifier)-1;

	class MaterialRequiredStreams;

	class IObjectDisplay;
	class IGeometryDisplay;

	class SmartHandle;
	class StreamFormatHandle;
	class RenderItemHandle;
	class RenderNodeHandle;
	class MaterialHandle;

	class IRenderStates;
	class IDisplayCallback;

	class BaseMaterialHandle;
	class MaterialRequiredStreams;

	struct ShadowParameters;

} } // namespaces
