/**********************************************************************
 *<
	FILE: ID3DGraphicsWindow.h

	DESCRIPTION: Direct3D Graphics Window Extension Interface class

	CREATED BY: Norbert Jeske

	HISTORY:

 *>	Copyright (c) 2000 - 2002, All Rights Reserved.
 **********************************************************************/

#pragma once

#ifndef _D3D9_GRAPHICS_WINDOW_H_
#  include "id3d9graphicswindow.h"
#endif

//
// This will always be the 'latest' D3D window, at the time the max SDK was 
// built.
//

#define D3D_GRAPHICS_WINDOW_INTERFACE_ID D3D9_GRAPHICS_WINDOW_INTERFACE_ID

typedef ID3D9GraphicsWindow ID3DGraphicsWindow;

