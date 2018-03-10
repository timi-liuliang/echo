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

#include "..\baseinterface.h"
#include "..\acolor.h"
#include "..\export.h"
#include "..\AnimPropertyID.h"
#include "..\matrix3.h"

class TexHandle;

namespace MaxSDK
{
	namespace Graphics
	{
		/** ISimpleMaterial provides any material or texture plugin the ability to display itself in the new Viewport Display system in 3ds Max 2012.
		ISimpleMaterial supports fast efficient handling of a basic phong style shader.  It supports all the usual surface 
		elements like Diffuse, Specular with the added ability to support two input textures for diffuse and opacity. If a developer needs finer control 
		over the visual appearance of the material then they should look to use the Advanced mode graphics API
		
		A developer	typically would not need to worry about many of the Set methods in the class as the viewport display system will translate 
		the host material calling MtlBase::GetDiffuse() and then using ISimpleMaterial::SetDiffuse to update the display definition.  
		Every material and texture map will have an ISimpleMaterial available as it is added as a property of the MtlBase.  To access the property
		use the following code 
		\code
		ISimpleMaterial *pISimpleMtl = (ISimpleMaterial *)GetProperty(PROPID_SIMPLE_MATERIAL);
		\endcode
		The only aspect of the material setup that is not fully automatic is the allocation of textures.  In general materials have a more 
		complicated requirement for texture display. An example would be diffuse and opacity display with different mapping channels.  Texture plugins on the other
		hand tend to be more simplistic. However, there are examples of a more complex setup as can be seen with the mix or composite maps.
		To support these more complex setups the developer must implement ITextureDisplay and implement SetupTextures. SetupTextures is called when ever a change
		in the material or texture is detected by the system. It is in this method the developer should assign the texture using SetTexture. This method also provides 
		a chance to override the automatic translation of the material parameters, however, it is not a requirement.  If ITextureDisplay is not implemented 
		the translation code will use GetActiveTexHandle() to access the texture resource, this is how texture maps such as Checker work.
		
		ISimpleMaterial also allows the material to define a draw style such as whether it is a solid or wireframe.  During translation the materials
		requirements will be checked.  For example the standard material can be forced into wireframe mode.  The translating code would look like this
		\code
		// assuming geomNode is a pointer to INode, the node currently being translated
		Mtl * mtl = geomNode->GetMtl();
		ULONG mtlReq = mtl->Requirements(0);
		if (mtlReq & MTLREQ_WIRE) {
				pISimpleMtl->SetFillMode(FillWireframe);
		}
		\endcode
		ISimpleMaterial is used to provide quick and approximate representations of materials in the viewport. More realistic material representations 
		can be created by writing custom real-time shader using the advanced mode graphics API
		\note
		All of the methods of this interface are implemented by the graphics driver layer, it is not designed to be derived by plug-ins. 
		\todo Provide links to Advanced materials when added to the SDK.
		*/
		class ISimpleMaterial : public BaseInterface
		{
		public:

			/** This is the fill mode used in both SetFillMode and GetFillMode.*/
			enum FillMode
			{
				FillPoint,			/*!< Point rendering style used for vertex ticks */
				FillWireframe,		/*!< Wireframe rendering style*/
				FillSolid,			/*!< Solid rendering style*/
			};

			/** This enum type tells the viewport rendering system the usage of each texture.*/
			enum MapUsage
			{
				UsageDiffuse = 0,	/*!< Diffuse channel*/
				UsageOpacity,		/*!< Opacity channel*/	
				// to add more
				MaxUsage,
			};

			/** Clones the instance of ISimpleMaterial.
			\return a pointer of new cloned ISimpleMaterial.
			*/
			virtual ISimpleMaterial* Clone() = 0;

			/** Specifies the fill mode of the material.
			\param[in] mode A member of ISimpleMaterial::FillMode.
			*/
			virtual void SetFillMode(ISimpleMaterial::FillMode mode) = 0;

			/** Returns the fill mode that the material is using.
			\return the fill mode.
			*/
			virtual ISimpleMaterial::FillMode GetFillMode() const = 0;

			/** Specifies the ambient color as an RGBA value.
			\param[in] color The ambient color.
			*/
			virtual void SetAmbientColor(const AColor& color) = 0;

			/** Returns ambient color that the material is using.
			\return the ambient color. 
			*/
			virtual const AColor& GetAmbientColor() const = 0;

			/** Specifies diffuse color as an RGBA value.
			If diffuse texture map exists, this value will be ignored.
			\param[in] color The diffuse color.
			*/
			virtual void SetDiffuseColor(const AColor& color) = 0;

			/** Returns diffuse color that the material is using.
			\return the diffuse color.
			*/
			virtual const AColor& GetDiffuseColor() const = 0; 

			/** Specifies the specular color as an RGBA value.
			\param[in] color The specular color.
			*/
			virtual void SetSpecularColor(const AColor& color) = 0;

			/** Returns specular color that the material is using.
			\return the specular color.
			*/
			virtual const AColor& GetSpecularColor() const = 0;

			/** Specifies the Emissive color as an RGBA value.
			\param[in] color The Emissive color.
			*/
			virtual void SetEmissiveColor(const AColor& color) = 0;
			
			/** Returns Emissive color that the material is using.
			\return the Emissive color.
			*/
			virtual const AColor& GetEmissiveColor() const = 0;

			/** Specifies the specular exponent of the material to be set.
			\param[in] power The specular amount.
			*/
			virtual void SetSpecularPower(float power) = 0;

			/** Returns specular power that the material is using.
			THe power value can be in the range of 0.0f and 1.0f
			\return the specular power.
			*/
			virtual float GetSpecularPower() = 0;

			/** Specifies the specular level.
			The specular level information is used to adjust the specular color.
			THe level value can be in the range of 0.0f and 10.0f
			\param[in] level The specular level value.
			*/
			virtual void SetSpecularLevel(float level) = 0;

			/** Returns specular level that the material is using.
			\return the specular level.
			*/
			virtual float GetSpecularLevel() const = 0;

			/** Specifies the self illumination mode.
			\param[in] bOn Sets whether Self Illumination Color is on or off.
			*/
			virtual void SetSelfIllumColorOn(bool bOn) = 0;

			/** Returns self illumination mode that the material is using.
			\return the self illumination mode. If TRUE, the SelfIllumColor is on. 
			*/
			virtual bool GetSelfIllumColorOn() const = 0;
			
			/** Set texture transform matrix
			\param[in] usage The texture usage
			\param[in] transform The transform matrix
			\return true if successful
			*/
			virtual bool SetTextureTransform(MapUsage usage, const Matrix3& transform) = 0;

			/** Get texture transform matrix
			\param[in] usage The texture usage
			\return the texture transform matrix
			*/
			virtual Matrix3& GetTextureTransform(MapUsage usage) = 0;

			/** Set texture with texture handle. \sa ITextureDisplay::SetupTextures()
			\param[in] pTexture A texture handle. If pTexture is NULL it will clear the texture.  This can 
			give finer control than using ClearTextures()
			\param[in] usage The texture usage
			\return True if successful 
			*/
			virtual bool SetTexture(TexHandle* pTexture, MapUsage usage) = 0;
			
			/** Get texture handle
			\param[in] usage The texture usage
			\return A pointer to texture handle
			*/
			virtual TexHandle* GetTexture(MapUsage usage) = 0;
			
			/** Clears all texture used by this material
			*/
			virtual void ClearTextures() = 0;
		};

	}
}

