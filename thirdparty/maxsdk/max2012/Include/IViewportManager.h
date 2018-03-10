/**********************************************************************
 *<
	FILE: IViewportManager.h

	DESCRIPTION:	Viewport Manager for loading up DXEffects and other 
					viewport properties

	CREATED BY:		Neil Hazzard

	HISTORY:		Created:  02/15/02
					Updated:  05/05/04	New interface added for 3ds max7
					Updated:  07/21/05  New interface added for 3ds max10
					Updated:  07/13/07  exposed some new viewport properties
					

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "custattrib.h"
#include "IHardwareMaterial.h"
#include "GetCOREInterface.h"
// forward declarations
class MtlBase;

#define VIEWPORT_SHADER_MANAGER_INTERFACE Interface_ID(0x5dbe33d6, 0x2e1b422b)
#define VIEWPORT_SHADER_MANAGER_INTERFACE2 Interface_ID(0x5a4d6672, 0x7d8a2857)
#define VIEWPORT_SHADER_MANAGER_INTERFACE3 Interface_ID(0x59b97216, 0x64d41c0d)

#define VIEWPORT_SHADER_CLIENT_INTERFACE Interface_ID(0x40c926b8, 0x7c3a66b7)
#define VIEWPORT_SHADER9_CLIENT_INTERFACE Interface_ID(0x40c926b7, 0x7c3a6347)

#define IDX_SHADER_MANAGER Interface_ID(0x6dce7429, 0x200169ac)

//! An Interface class to access the Viewport Manager
/*! This Interface allows access to the various components of the Viewport Manager.  THis includes finding out
how many %effects are registered with the system
*/
class IViewportShaderManager : public FPMixinInterface {
	public:

		enum {	get_num_effects, get_active_effect,	is_effect_active, is_manager_active,
		get_effect_name,set_effect,activate_effect};

		FPInterfaceDesc* GetDesc();    

		/*! Get the number of effects registered with the system
		\return The total number of effects
		*/
		virtual int GetNumEffects()=0;

		/*! Get the current active effect.  This effect will implement the IDXDataBridge classes
		\return A referenceTarget implementing the effect
		*/
		virtual ReferenceTarget* GetActiveEffect()=0;

		/*! Access the UI name for the effect
		\param i An index of the effect whose name is required
		\return A MCHAR containing the name
		*/
		virtual MCHAR * GetEffectName(int i)=0;

		/*! Set the actual viewport effect to be used.
		\param i An index of the effect to set.
		\return A pointer to the effect
		*/
		virtual ReferenceTarget * SetViewportEffect(int i)=0;

		/*! Turns on the effect based on the material and state supplied.  THis sets the internal flag on the material
		that tells the system that a DX shader is present
		\param *mtl THe material to activate
		\param State A boolean value that defines the state of the flag
		*/
		virtual void ActivateEffect(MtlBase * mtl, BOOL State)=0;


	};
//! A new interface for 3dsmax 7
/*! This extends the current interface, and provides support for the new UI options on the Viewport Manager
*/
class IViewportShaderManager2 : public IViewportShaderManager
{
public:
	//! Save the current effect based on the Standard Material as an effect file
	/*!
	\param *fileName The name of the file to save
	\return  The success of the operation
	*/
	virtual bool SaveFXFile(MCHAR * fileName)=0;

	/*! Is the DX Display of the Standard Material enabled
	\return TRUE/FALSE
	*/
	virtual bool IsDxStdMtlEnabled()=0;

	/*! Access to set the state of the DX Display of the Standard Material flag
	\param state The state of the flag - TRUE to set
	*/
	virtual void SetDxStdMtlEnabled(bool state)=0;

	/*! The name of the current effect as displayed in the UI
	\return The name of the effect
	*/
	virtual MCHAR * GetActiveEffectName() = 0;
};

//! A new interface for 3dsmax 10
/*! Extends IViewportShaderManager2
This interface is implemented by 3ds Max, more specifically by the DirectX 
Manager and Mental Ray custom attributes of materials. Client code can get 
this interface from a material with the following code:
\code
Material* pMaterial = // initialize material pointer
ICustAttribContainer* pCAContainer = pMaterial->GetCustAttribContainer();
BaseInterface* pBaseIfc = pCAContainer->FindCustAttribInterface(VIEWPORT_SHADER_MANAGER_INTERFACE3);
IViewportShaderManager3* pViewportShaderMgr = static_cast<IViewportShaderManager3*>(pBaseIfc);
\endcode
*/
class IViewportShaderManager3 : public IViewportShaderManager2
{
public:
	/*! GetActiveEffect and GetActiveEffectName returns information of currently 
	selected effect, however, the effect may still not be "enabled". To get the 
	enable/disable state of the effect, use this function.
	\return The enable/disable state of the effect
	*/
	virtual bool IsCurrentEffectEnabled() = 0;

	/*! Access to set the enable/disable state of the current selected effect.
	\param enabled The state of the flag - TRUE to set
	\return if current effect is enabled successfully, return true, otherwise 
		return false.
	*/
	virtual bool SetCurrentEffectEnabled(bool enabled) = 0;

	/*! Is the DX Display of the Material is supported at this moment.
	\return true means that we could turn directX rendering of the material on. 
		otherwise return false.
	*/
	virtual bool IsDxStdMtlSupported() = 0;

	/*! Is saving Standard Material as .fx file is supported at this moment.
	\return true means that we could save current material as a .fx file. 
		otherwise return false.
	*/
	virtual bool IsSaveFxSupported() = 0;

	/*! Is current selected effect is supported.
	\return true means that we could enable current effect. otherwise if current 
		effect can not be enabled, return false.
	*/
	virtual bool IsCurrentEffectSupported() = 0;

	/*! 
	\return owner material of this interface
	*/
	virtual MtlBase* FindOwnerMaterial() = 0;
};

//! Viewport DXEffect interface
/*! A Direct X 8.1 shader should implement this class.  It is used by the system to get information from the shader
If a material implements this class, then all it needs to do is pass back a pointer in the GetInterface implementation.  
All the other methods can simply be a no op.
*/
/*! \sa  : Class IDXShaderManagerInterface, Class IViewportShaderManager, Class IHardwareMaterial\n\n
class IDXDataBridge: public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This is the interfaced used by the Viewport Shader plugins. For the manager to
load up a Viewport Shader it must implement this class.\n\n
As well as implementing the dialog loading it also provides a link to the GFX.
This means the developer is free use IDX8VertexShader and IDX8PixelShader
interfaces, for more advanced shaders. Examples of these shaders are\n\n
<b>/MAXSDK/SAMPLES/HardwareShaders/Cubemap/Cubemap.cpp</b> and\n\n
<b>/MAXSDK/SAMPLES/HardwareShaders/Membrane/Membrane.cpp</b>\n\n
<b>An important point to make with the usage of these plugins is in the way the
ViewportManager loads them up. To determine whether the plugin is a shader it
checks the Category in the ClassDesc. This must return "DXViewportEffect"
otherwise it will not be listed in the database. The reason for this is that
the Manager supports deferred loading, and this is one way of checking the
plugin without requiring a full DLL load.</b>\n\n
For an example of this class in use by <b>ViewportShader</b> plug-ins see
<b>/MAXSDK/SAMPLES/HardwareShaders/LightMap/Lightmap.cpp</b>\n\n
If the Viewport Shader plugin hosts any animated parameters that will be viewed
in the Trackview then it is important the plugin implements the following
Reference Message \n\n
\code
RefResult LightMap::NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message )
{
	switch (message) {
//this allows the manager to control the TV better.
		case REFMSG_WANT_SHOWPARAMLEVEL:
		{
			BOOL * pb = (BOOL*)(partID);
			*pb = TRUE;
			return REF_HALT;
		}
	}
	return(REF_SUCCEED);
}
\endcode 
This will make sure that the Parameters show up under the Viewport Manager in
the correct format.\n\n
All methods of this class are virtual.\n\n
   */
class IDXDataBridge : public BaseInterface
{
public:
	/*! \remarks The returns the unique ID for this interface. By default it
	will return <b>VIEWPORT_SHADER_CLIENT_INTERFACE</b> */
	virtual Interface_ID	GetID() { return VIEWPORT_SHADER_CLIENT_INTERFACE; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }
	/*! The Viewport Manager will call this to display the dialog (if needed)
	\param hWnd The parent window handle
	\param *imp A pointer to IMtlParam - see the main max SDK for info on this class
	\return A new ParamDlg pointer which the Viewport Manager will uses as a child of its window
	*/
	/*! \remarks This allows the Viewport shader to create a UI. This will be
	added as a child of the Viewport Manager.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The window handle to the parent window\n\n
	<b>IMtlParams * imp</b>\n\n
	This can be used in the call to CreateParamsDlg.\n\n
	  */
	virtual ParamDlg * CreateEffectDlg(HWND hWnd, IMtlParams * imp)= 0;

	//! This is no longer used
	/*! \remarks Currently this method is not used\n\n
	  */
	virtual void DisableUI()=0;

	/*! Get the UI name of the effect - this is only used for Viewport MAnager effects - not materials
	\return A string containing the name
	*/
	/*! \remarks This allows the plugin to return a name to be used in max.
	This may be extended for future use, but currently it is only used in the
	trackview. */
	virtual MCHAR * GetName()=0;

	/*! For effects that will be using the fixed function pipeline.  This provided access to the IHArdwareMaterial
	allowing access to DirectX FFP.
	\param *pHWMtl A pointer to the internal hardware material.
	\param *pMtl A pointer to the parent Material
	*/
	/*! \remarks This allows the plugin to use methods of
	Class IHardwareMaterial to
	perform custom drawing of an object. This could be special texture setups
	and control over the Render States and Texture Stages. If the developer
	needs finer control over the object, then the r4 interfaces IDX8VertShader
	can be used instead, and this function will not do anything. This method
	will be called whenever the object needs drawing. For a detailed example of
	this usage, please look at the Lightmap sample in the sdk\n\n

	\par Parameters:
	<b>IHardwareMaterial * pHWMtl</b>\n\n
	Provides access to various drawing techniques\n\n
	<b>Mtl * pMtl</b>\n\n
	A pointer to the parent material. */
	virtual void SetDXData(IHardwareMaterial * pHWMtl, Mtl * pMtl)=0;
};


// If you are creating a DX9 based DXEffects then you need to implement this interface 
class IDX9DataBridge : public IDXDataBridge
{
public:
	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }
	virtual Interface_ID	GetID() { return VIEWPORT_SHADER9_CLIENT_INTERFACE; }

	//! Versioning
	/*! The system uses this to determine whether the effect is compatible to the current GFX loaded on MAX
	\return 8.1 or 9.0 etc...
	*/
	virtual float GetDXVersion() = 0;
	
};

/*! An Interface that provides access to the actual manager.  The manager is responsible for displaying the Viewport Manager which is 
implemented as a Cust Attribute on the material
*/
/*! <b>Class IDXShaderManagerInterface</b>\n\n
\sa  Class IViewportShaderManager\n\n
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
The class provides access to the Viewport Manager feature in release 5. The
viewport manager controls the loading of Viewport Shaders in 3ds Max and is
displayed on each material. The class provides methods to query the manager to
find out whether it is visible or active, and also to retrieve the active
shader.\n\n
The viewport manager ONLY works in DirectX mode. It can however be visible in
Heidi and OpenGL, so as to provide the artist feedback when loading someone
else's file.\n\n
There is a global method you can call to get access to the DX manager.\n\n
<b>IDXShaderManagerInterface* GetDXShaderManager()</b>\n\n
 */
class IDXShaderManagerInterface : public FPStaticInterface
{
	public:
		/*! \remarks This method will check the Material passed into find out
		whether it contains the ViewportManager Custom Attribute. If it does it
		will return its pointer, otherwise it will be NULL. This pointer can
		safely be cast to <b>IViewportShaderManager.</b>\n\n

		\par Parameters:
		<b>MtlBase* mtl</b>\n\n
		The material to search for the Viewport Manager.\n\n
		  */
		virtual CustAttrib* FindViewportShaderManager (MtlBase* mtl)=0;
		/*! \remarks This method will add the ViewportManager custom atrribute
		to the material supplied. If successful it will return the newly
		created custom attribute. This pointer can safely be cast to
		<b>IViewportShaderManager.</b>\n\n

		\par Parameters:
		<b>MtlBase* mtl</b>\n\n
		The material to add the Viewport Manager to\n\n
		  */
		virtual CustAttrib* AddViewportShaderManager(MtlBase * mtl)=0;
		
		/*! \remarks This method allows the system to either show or hide the
		ViewportManager. This will physically remove it from the Material
		Editor, whoever the manager will still exist on the material\n\n

		\par Parameters:
		<b>BOOL show</b>\n\n
		The value to set the visible state of the manager.\n\n
		   */
		virtual void	SetVisible(BOOL show=TRUE)=0;
		/*! \remarks This method will return the actual visible state of the
		manager */
		virtual BOOL	IsVisible()=0;
	
};

inline IDXShaderManagerInterface* GetDXShaderManager() { return (IDXShaderManagerInterface*)GetCOREInterface(IDX_SHADER_MANAGER); }





