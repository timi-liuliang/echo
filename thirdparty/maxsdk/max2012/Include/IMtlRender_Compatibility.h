 /**********************************************************************
 
	FILE:           IMtlRender_Compatibility.h

	DESCRIPTION:    Public interface for determining compatibility
					between a material/texmap and a renderer.

					The interface also allows 3rd party material/map plugins to
					use custom icons in the material/map browser.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        Created 14 April 2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "baseinterface.h"
#include <CommCtrl.h> // for HIMAGELIST
#include "plugapi.h"

// forward declarations
class Renderer;
class MtlBase;

#define IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID Interface_ID(0x5537445b, 0x70a97e02)
#define IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID Interface_ID(0x25d24114, 0xdbe505f)

//==============================================================================
// class IMtlRender_Compability_MtlBase
/*! \sa Class IMtlRender_Compatibility_Renderer, Class ClassDesc2, Class Mtl, Class Texmap
 
   \par Description:
   This interface is used to determine whether a material/map flags itself as
   being compatible with a specific renderer plugin. If the material/map flags
   itself as being compatible with a renderer, then it is deemed compatible with
   that renderer regardless of what class IMtlRender_Compatibility_Renderer
   might say.

   \par Implementation:
   To be inheritted by a ClassDesc or ClassDesc2 of a Texmap or Mtl plugin.
   The child class needs to call Init(*this) in its constructor.

   \par Default compatibility: 
   If neither the renderer nor the material/map implements
   the compatibility interface, they are considered compatible by default.

   \par Material/map browser icon: 
	The interface also provides a way of defining an 
   icon that appears in the material browser - GetCustomMtlBrowserIcon. This can 
   be used to provide an Enable\Disabled color. In mental ray enabled uses a yellow 
   icon, the DirextX 9 Shader uses a pink icon. Developers can use this to highlight 
   special materials or textures.
*/
//==============================================================================
class IMtlRender_Compatibility_MtlBase : public FPStaticInterface {

public:

	/*! \remarks Initialization method. MUST be called from the constructor of the child class. i.e. "Init(*this);". 
		\param classDesc - Reference to sub-class instance */
	void Init(ClassDesc& classDesc);

	/*! \remarks Returns whether this material/map is compatible with the given renderer.
		(Use the class ID of the renderer to determine compatibility).
		\param rendererClassDesc - Class descriptor of a Renderer plugin */
	virtual bool IsCompatibleWithRenderer(ClassDesc& rendererClassDesc) = 0;

	/*! \remarks Provides custom icons for the material/map browser. Returns true if custom
		icons are provided.\n\n
		When using this method, the developer should keep a single instance of the imagelist used, 
		and it is the developers responsibility to release the memory
		\param hImagelist - The image list from which the icons are extracted. The images should have a mask.
		\param inactiveIndex - Index (into image list) of icon to be displayed when the material/map has the "Show Maps in Viewport" flag turned OFF.
		\param activeIndex - Index (into image list) of icon to be displayed when the material/map has the "Show Maps in Viewport" flag turned ON.
		\param disabledIndex - Index (into image list) of icon to be displayed when the material/map is NOT COMPATIBLE with the current renderer.
		\return true if custom icons are provided */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual bool GetCustomMtlBrowserIcon(
		HIMAGELIST& hImageList,
		int& inactiveIndex,
		int& activeIndex,
		int& disabledIndex
	) 
	{ 
		return false; 
	}
	#pragma warning(pop)
};

// Given the class descriptor of a Mtl/Texmap plugin, this returns its compatibility interface (if it exists).
inline IMtlRender_Compatibility_MtlBase* Get_IMtlRender_Compability_MtlBase(ClassDesc& mtlBaseClassDesc) {

	return static_cast<IMtlRender_Compatibility_MtlBase*>(mtlBaseClassDesc.GetInterface(IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID));
}

//==============================================================================
// class IMtlRender_Compatibility_Renderer
/*! \sa IMtlRender_Compatibility_MtlBase, Class ClassDesc2, Class Mtl, Class Texmap
	\par Description:
	This interface is used to determine whether a Renderer flags itself as
	being compatible with a specific material/map plugin. If the Renderer flags
	itself as being compatible with a material/map, then it is deemed compatible with
	that material/map regardless of what class IMtlRender_Compatibility_MtlBase
	might say.

	\par Implementation instructions:
	To be inheritted by a ClassDesc or ClassDesc2 of a Renderer plugin.
	The child class needs to call Init(*this) in its constructor.
 
	\par Default compatibility: 
	If neither the renderer nor the material/map implements
	the compatibility interface, they are considered compatible by default.
*/
//==============================================================================
class IMtlRender_Compatibility_Renderer : public FPStaticInterface  {

public:

	/*! \remarks Initialization method. MUST be called from the constructor of the child class. i.e. "Init(*this);".
		\param classDesc - Reference to sub-class instance */
	void Init(ClassDesc& classDesc);

	/*! \remarks Returns wethere this material/map is compatible with the given renderer.
		(Use the class ID of the renderer to determine compatibility).
		\param mtlBaseClassDesc	- Class descriptor of Mtl or Texmap plugin
		\return true if this renderer is compatable with the given material */
	virtual bool IsCompatibleWithMtlBase(ClassDesc& mtlBaseClassDesc) = 0;
};

// Given the class descriptor of a Renderer plugin, this returns its compatibility interface (if it exists).
inline IMtlRender_Compatibility_Renderer* Get_IMtlRender_Compatibility_Renderer(ClassDesc& rendererClassDesc) {

	return static_cast<IMtlRender_Compatibility_Renderer*>(rendererClassDesc.GetInterface(IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID));
}

////////////////////////////////////////////////////////////////////////////////
// bool AreMtlAndRendererCompatible(ClassDesc&, ClassDesc&)
/*!
	Returns whether the given Mtl/Texmap plugin is compatible with the given
	Renderer plugin. Always use this function to determine compatiblity.
*/
inline bool AreMtlAndRendererCompatible(ClassDesc& mtlBaseClassDesc, ClassDesc& rendererClassDesc) {

	IMtlRender_Compatibility_MtlBase* mtlBaseCompatibility = Get_IMtlRender_Compability_MtlBase(mtlBaseClassDesc);
	IMtlRender_Compatibility_Renderer* rendererCompatibility = Get_IMtlRender_Compatibility_Renderer(rendererClassDesc);

	if((mtlBaseCompatibility == NULL) && (rendererCompatibility == NULL)) {
		// No compatibility info: compatible by default
		return true;
	}
	else if((mtlBaseCompatibility != NULL) && mtlBaseCompatibility->IsCompatibleWithRenderer(rendererClassDesc)) {
		// Material says it's compatible with the renderer: compatible
		return true;
	}
	else if((rendererCompatibility != NULL) && rendererCompatibility->IsCompatibleWithMtlBase(mtlBaseClassDesc)) {
		// Renderer says it's compatible with the material: compatible
		return true;
	}
	else {
		// Neither material nor renderer says it's compatible: incompatible
		return false;
	}
}

//==============================================================================
// class IMtlRender_Compability_MtlBase inlined methods
//==============================================================================

inline void IMtlRender_Compatibility_MtlBase::Init(ClassDesc& classDesc) {

	LoadDescriptor(IMTLRENDER_COMPATIBILITY_MTLBASE_INTERFACEID, _M("IMtlRender_Compability_MtlBase"), 0, &classDesc, 0, end);
}

//==============================================================================
// class IMtlRender_Compatibility_Renderer inlined methods
//==============================================================================

inline void IMtlRender_Compatibility_Renderer::Init(ClassDesc& classDesc) {

	LoadDescriptor(IMTLRENDER_COMPATIBILITY_RENDERER_INTERFACEID, _M("IMtlRender_Compability_Renderer"), 0, &classDesc, 0, end);
}

