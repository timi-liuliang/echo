//
// Copyright [2009] Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
//

#pragma once
#include "../baseinterface.h"
#include "../imtl.h"
#include "ISimpleMaterial.h"


/** Interface ID of ITextureDisplay */
#define ITEXTURE_DISPLAY_INTERFACE_ID Interface_ID(0x363f6c8b, 0x14500438)

namespace MaxSDK { namespace Graphics {

/** Used for creating texture handles (TexHandle) and updating texture data managed 
by the viewport display system. This class is implemented by the system and an 
instance of it is passed as parameter to ITextureDisplay::SetupTextures().
\sa  Class TexHandleMaker, Class ITextureDisplay.
*/
class DisplayTextureHelper : public TexHandleMaker
{
public:
	/** Updates texture information used by the viewport display system.  
	It extracts mapping channel and texture transform information from the provided Texmap.
	Client code needs to call this method in its implementation of 
	ITextureDisplay::SetupTextures() for each texture map of a material or sub-texture 
	of a texture map plug-in.
	\param t The time at which the texture information is updated.
	\param mapUsage The map usage which specifies which of the internal texture information will be updated.
	\param texMap The texture map representing the source for the texture information update.
	*/
	virtual void UpdateTextureMapInfo(TimeValue t, ISimpleMaterial::MapUsage mapUsage, Texmap* texMap) = 0;
};


/** This interface enables material and texture map plugins to display textures in the new viewport 
	display system in 3ds Max 2012. Developers override the ITextureDisplay::SetupTextures() method to assign 
	a texture resource to the ISimpleMaterial managed by the material or texture map plug-in.

	In general materials have a more complicated requirement for texture display. An example would be diffuse and opacity display 
	with different mapping channels.  Texture plugins on the other hand tend to be more simplistic. However, there are examples of a more complex setup 
	as can be seen with the mix or composite maps.  To support these complex setups a plugin must implement ITextureDisplay, otherwise
	during translation GetActiveTexHandle() will be called.

	The texture resource is represented by the TexHandle class and can be created using TexHandleMaker::CreateHandle().  
	The TexHandle instance lifetime is managed by the plugin. If the texture data managed by a plug-in (e.g. texture 
	related parameters) becomes invalid then the plug-in must deleted the TexHandle and to call ISimpleMaterial::ClearTextures(). It is 
	possible to clear a single texture resource by calling SetTexture() with NULL and the appropriate map usage.
	\note See Stdmtl2.cpp for an example implementation of SetupTextures()
	\sa ISimpleMaterial
	\todo Show source code example
*/
class ITextureDisplay : public BaseInterface
{
public:
	/** Get the unique ID for this interface. 
	By default it will return ITEXTURE_DISPLAY_INTERFACE_ID*/
	virtual Interface_ID GetID() { return ITEXTURE_DISPLAY_INTERFACE_ID; }

	/** Allows for setting up the textures of a ISimpleMaterial.
		The automatic translation of material and texture map plug-ins can be 
		complemented by implementing this method to setup the texture maps of a 
		material or texture map plug-in by calling ISimpleMaterial::SetTexture().
		This method can also override in its implementation the automatic translation
		of all material parameters (diffuse color, etc).  
		Texture map plug-ins should only setup their textures with ISimpleMaterial::SetTexture 
		as they are displayed without lighting or other surface details. 
		Texture maps assigned to a ISimpleMaterial will be used by both the viewport 
		and the hardware renderer.
		This method is called by 3ds Max when the parameters of a material or texture map change.
		\param t The time at which the textures need to be setup
		\param updater the helper object that needs to be used to update the display
		representation of the textures */
	virtual void SetupTextures(TimeValue t, DisplayTextureHelper &updater) = 0;
};

} } //namespace 
