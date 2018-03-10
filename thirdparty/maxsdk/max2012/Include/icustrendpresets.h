/////////////////////////////////////////////////////////////////////////
//
//
//	Render Preset Interface
//
//	Created 5/14/2003	Tom Burke
//

#pragma once

// includes
#include "maxheap.h"
#include "sfx.h"
#include "itargetedio.h"
#include "irenderpresets.h"

//////////////////////////////////////////////////////////////
// Class ICustomRenderPresets
/*! 
\sa Class Renderer
\par Description:
	Render Presets compatibility interface:\n\n
	used by renderers to support renderer specific presets.
	If the renderer does not support this interface, the entire
	renderer will be saved.
*/
//////////////////////////////////////////////////////////////
class ICustomRenderPresets: public MaxHeapOperators
{
public:
	virtual int  RenderPresetsFileVersion() = 0;
	virtual BOOL RenderPresetsIsCompatible( int version ) = 0;

	virtual MCHAR * RenderPresetsMapIndexToCategory( int catIndex ) = 0;
	virtual int     RenderPresetsMapCategoryToIndex( MCHAR* category ) = 0;

	virtual int RenderPresetsPreSave( ITargetedIO * root, BitArray saveCategories ) = 0;
	virtual int RenderPresetsPostSave( ITargetedIO * root, BitArray loadCategories ) = 0;
	virtual int RenderPresetsPreLoad( ITargetedIO * root, BitArray saveCategories ) = 0;
	virtual int RenderPresetsPostLoad( ITargetedIO * root, BitArray loadCategories ) = 0;
};

