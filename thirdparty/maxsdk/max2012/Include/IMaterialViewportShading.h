/**********************************************************************
 *<
    FILE: IMaterialViewportShading.h

    DESCRIPTION: Interface definitions for Viewport Shading Support

    CREATED BY: Qilin.Ren

    HISTORY: March 2007

 *> Copyright (c) 2007, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "imtl.h"


#define IID_MATERIAL_VIEWPORT_SHADING Interface_ID(0x3ae06719, 0x65847f74)

//! A new interface for 3ds Max 2008
/*! This interface is used for manipulating material shading model in the viewport. A material
would implement this interface so as to control the "Show map in viewport" (SMIV) button.  In 3dsmax 2008
the SMIV button can now show as a flyoff, for regular shading (Blue) and Hardware shading (Pink).  If 
a material supports a Hardware representation then it can implement this interface and return the correct
level support in IsShadingModelSupported.  This will mean the UI is reflective of the support available.
This interface DOES NOT provide any support for the actual Hardware implementation - purely the UI synchronisation.  
An implementation for this interface can be found at \maxsdk\samples\materials\StdMaterialViewportShading.cpp where
it support the case where the material deals with plugin hardware shaders as well.
Currently in 3ds max 2008, only two materials implement a Hardware version, and they are the Standard Material and 
Mental Images's Architectural and Design Material.
*/
class IMaterialViewportShading : public BaseInterface
{
public:
	/// This enumeration defines how the material is behaving in viewport
	enum ShadingModel
	{
		/// the material is rendered in standard way - original Viewport rendering.
		Standard, 
		/// the material is rendered by using hardware shaders.
		Hardware, 
		/// other render types, such as plugin effect shading.
		OtherTypes, 
	};

public:
	// from BaseInterface
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);
	CoreExport virtual Interface_ID GetID();

protected:
	/*! Does the material support a specific shading model in viewport. This will be used to enable\disable the SMIV options.
	\param model The shading model, for now it must be Standard or Hardware.
	\return if the shading model is supported, return true; otherwise return false
	*/
	virtual bool IsShadingModelSupported(ShadingModel model) const = 0;

	/*! Used to query the material for its current level of viewport rendering
	\return current shading model.
	*/
	virtual ShadingModel GetCurrentShadingModel() const = 0;

	/*! Set the shading model of the material in viewport.
	\param model The shading model, must be Standard or Hardware.
	\return If the shading model is set successfully, return true; otherwise return false.
	*/
	virtual bool SetCurrentShadingModel(ShadingModel model) = 0;

	/*! The map levels needs to be defined by the material, it will tell the 
	various shader compilers how far to go in the search of materials/maps.  Most materials 
	will stop searching after one level, as the shader becomes too complex.  For example specifiying
	1 will mean the first map found will be used e.g a BitmapTex in a diffuse slot.  The compiler will not search any 
	further for maps to support. In this case it will probably bake the submaps to textures and will not try to generate	
	shader code for these submaps.
	\return How deep the shader compilers will go in the search of material/Maps in the  tree.
	*/
	virtual int GetSupportedMapLevels() const = 0;

public:
	// Helper functions for accessing the IMaterialViewportShading interface on Materials. These
	// are exported through Core.dll

	/*! Does the material support a specific shading model in viewport. This will be used to enable\disable the SMIV options.
	\param material The material you want to test. If the material 
	does not support IMaterialViewportShading interface, then only standard shading model is supported.
	\param model The shading model, for now it must be Standard or Hardware.
	\return if the shading model is supported, return true; otherwise return false
	*/
	CoreExport static bool IsShadingModelSupported(
		const MtlBase& material, 
		ShadingModel model);

	/*! Used to query the material for its current level of viewport rendering
	\param material The material you want to get shading model from. If the material 
	does not support IMaterialViewportShading interface, then the function will return Standard shading model.
	\return current shading model.
	*/
	CoreExport static ShadingModel GetCurrentShadingModel(
		const MtlBase& material);

	/*! Set the shading model of the material in viewport and update the UI of material editor.
	\param material The material you want to set its shading model. If the material does not 
	support the shading model, the function will fail.
	\param model The shading model, must be Standard or Hardware.
	\return If the shading model is set successfully, return true; otherwise return false.
	*/
	CoreExport static bool SetCurrentShadingModel(
		MtlBase& material, 
		ShadingModel model);

	/*! The map levels needs to be defined by the material, it will tell the 
	various shader compilers how far to go in the search of materials/maps.  Most materials 
	will stop searching after one level, as the shader becomes too complex.  For example specifiying
	1 will mean the first map found will be used e.g a BitmapTex in a diffuse slot.  The compiler will not search any 
	further for maps to support. In this case it will probably bake the submaps to textures and will not try to generate	
	shader code for these submaps.
	\param material The material you want to get map levels from. If the material does not 
	support IMaterialViewportShading interface, the function returns 1.
	\return How deep the shader compilers will go in the search of material/Maps in the tree.
	*/
	CoreExport static int GetSupportedMapLevels(
		const MtlBase& material);
};


