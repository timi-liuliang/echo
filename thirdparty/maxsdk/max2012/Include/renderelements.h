/////////////////////////////////////////////////////////////////////////
//
//
//	Render Element Plug-Ins
//
//	Created 4/15/2000	Kells Elmquist
//

#pragma once

// includes
#include "maxheap.h"
#include "sfx.h"

// forward declarations
class IllumParams;
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}


#define BEAUTY_RENDER_ELEMENT_CLASS_ID			0x00000001

#define SHADOWS_RENDER_ELEMENT_CLASS_ID			0x00000008
#define Z_RENDER_ELEMENT_CLASS_ID				0x0000000c
#define ALPHA_RENDER_ELEMENT_CLASS_ID			0x0000000d
#define LIGHTING_RENDER_ELEMENT_CLASS_ID		0x00000011
#define MATERIALID_RENDER_ELEMENT_CLASS_ID      0x00000014
#define OBJECTID_RENDER_ELEMENT_CLASS_ID		0x00000015
#define ILLUMINANCE_RENDER_ELEMENT_CLASS_ID     0x00000017


// Returned by a RenderElement when it is asked to put up its rollup page.
typedef SFXParamDlg IRenderElementParamDlg;


//////////////////////////////////////////////////////////////
//
//		RenderElement base interface
//
//		class SpecialFX declared in maxsdk/include/render.h
//
/*! \sa  Class SpecialFX, Class PBBitmap, Class ISave,  Class ILoad, \ref Reference_Messages, 
<a href="ms-its:3dsmaxsdk.chm::/render_elements.html">Render Elements</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the interface that must be supported by all render elements whether
they support the 3ds Max renderer or some other renderer. The UI in the render
dialog uses this interface exclusively to control the element.\n\n
The methods are almost all state-setting methods, with ones that are settable
by the UI provided by both sets and gets. A sample plugin of a Render Element
can be found in the SDK samples; <b>/MAXSDK/SAMPLES/RENDER/RENDERELEMENTS</b>.
 */
#pragma warning(push)
#pragma warning(disable:4100)
class IRenderElement : public SpecialFX
{
public:
	// set/get element's enabled state
	/*! \remarks This method enables or disables the Render Element.
	\par Parameters:
	<b>BOOL enabled</b>\n\n
	Set to TRUE in order to enable the Render Element. FALSE to disable it. */
	virtual void SetEnabled(BOOL enabled)=0;
	/*! \remarks This method returns TRUE if the Render Element is enabled,
	otherwise FALSE. */
	virtual BOOL IsEnabled() const = 0;

	// set/get element's filter enabled state
	/*! \remarks Each active render element has the option of either using the
	current AA filter or simple blending within the pixel. This method will set
	the internal filter enable to the value of parameter <b>filterEnabled</b>.
	\par Parameters:
	<b>BOOL filterEnabled</b>\n\n
	Set to TRUE in order to enable. FALSE to disable it. */
	virtual void SetFilterEnabled(BOOL filterEnabled)=0;
	/*! \remarks This method returns the current state of whether the AA
	filter is enabled.
	\par Parameters:
	This method returns TRUE if filters for the Render Element are enabled,
	otherwise FALSE. */
	virtual BOOL IsFilterEnabled() const =0;

	// is this element to be blended for multipass effects?
	/*! \remarks This method returns whether this element type should be
	blended during multipass effects.\n\n
	When multipass camera effects such as depth of field are used in a
	rendering, each of the separate elements may be blended into a final bitmap
	like the composite color, or they may be "frozen" after the first pass.
	Blending is not appropriate for some elements, like z-depth. This is
	typically a query only, it is unlikely that this will need to be turned on
	\& off.
	\return  TRUE if blending during multipass effects, otherwise FALSE. */
	virtual BOOL BlendOnMultipass() const =0;

	/*! \remarks This method returns TRUE if atmospheric effects for the
	Render Element are enabled, otherwise FALSE. */
	virtual BOOL AtmosphereApplied() const =0;

	/*! \remarks This method returns TRUE if shadows for the Render Element
	are enabled, otherwise FALSE. */
	virtual BOOL ShadowsApplied() const =0;

	// set/get element's name (as it will appear in render dialog)
	/*! \remarks This method sets the Render Element's name as it appears in
	the render dialog.
	\par Parameters:
	<b>MCHAR* newName;</b>\n\n
	The name for the Render Element. */
	virtual void SetElementName( const MCHAR* newName )=0; 

	// deprecated method - implement SetElementName(const MCHAR *newName) 
	virtual void SetElementName( MCHAR* newName ) { SetElementName(const_cast<const MCHAR*>(newName)); }

	/*! \remarks This method returns a string representing the Render
	Element's name as it appears in the render dialog. */
	virtual const MCHAR* ElementName() const =0;

	/*! \remarks This method allows you to set the bitmapinfo/bitmap to use for
	the Render Element.\n\n
	Each render element has an output bitmap. We use the pb2 style bitmap as it
	contains the pathname as well as the bitmap \& bitmapInfo structures needed by
	windows. The bitmap is created by the render element manager, then held by the
	element until it's not needed. These are the calls that set \& get the elements
	bitmap.
	\par Parameters:
	<b>PBBitmap* \&pPBBitmap</b>\n\n
	The pointer to the PBBitmap reference. */
	virtual void SetPBBitmap(PBBitmap* &pPBBitmap) const =0;
	/*! \remarks This method allows you to get the bitmapinfo/bitmap that is used
	for the Render Element.
	\par Parameters:
	<b>PBBitmap* \&pPBBitmap</b>\n\n
	The pointer to the PBBitmap reference which was retrieved. */
	virtual void GetPBBitmap(PBBitmap* &pPBBitmap) const =0;

	// this is the element specific optional UI, which is a rollup in the render dialog
	/*! \remarks Each render element may define a rollup that will be
	displayed in the render dialog when that element is selected. Most current
	render elements do not provide their own rollups, but some like z-depth and
	blend do. This method creates the elements parameter rollup and return it's
	pointer to the system. If no rollup is supported, NULL should be returned.
	\par Parameters:
	<b>IRendParams *ip</b>\n\n
	A pointer to the IRendParams data.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual IRenderElementParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }

	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// IRenderElement has secondary dialogs that don't have the IRenderElement as their 'thing'.
	// Called once for each secondary dialog, for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	/*! \remarks Implement this method if you are using the 
	ParamMap2 AUTO_UI system and the
	IRenderElement has secondary dialogs that don't have the IRenderElement as
	their 'thing'. Called once for each secondary dialog, for you to install
	the correct 'thing'. This method will set the "thing" of a secondary
	dialog.
	\par Parameters:
	<b>IRenderElementParamDlg* dlg</b>\n\n
	The pointer to the parameter dialog.
	\return  TRUE if you process the dialog, otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; };</b> */
	virtual BOOL SetDlgThing(IRenderElementParamDlg* dlg) { return FALSE; }

	// ---------------------
	// from class RefMaker
	// ---------------------
	// it is critical for merging that this code is called at the start of a plug-in's save and load methods.
	// SpecialFX's base implementation saves/loads SpecialFX::name, which is used to populate the 'Merge Render Elements'
	// dialog box. if a plugin re-implements this function, it should first call IRenderElement::Save(iSave)
	// or IRenderElement::Load(iLoad)
	/*! \remarks This method handles saving the plugin data. it is critical
	for merging that this code is called at the start of a plug-in's save and
	load methods. SpecialFX's base implementation saves/loads SpecialFX::name,
	which is used to populate the 'Merge Render Elements' dialog box. if a
	plugin re-implements this function, it should first call
	<b>RenderElement::Save(iSave)</b> or <b>IRenderElement::Load(iLoad)</b>
	\par Parameters:
	<b>ISave *iSave</b>\n\n
	You may use this pointer to call methods of ISave to write data.
	\return  One of the following values: <b>IO_OK</b>, <b>IO_ERROR</b>.
	\par Default Implementation:
	<b>{</b>\n\n
	<b> name = ElementName();</b>\n\n
	<b> return SpecialFX::Save(iSave);</b>\n\n
	<b>}</b> */
	IOResult Save(ISave *iSave)
	{
		name = ElementName();
		return SpecialFX::Save(iSave);
	}
	/*! \remarks This method handles loading the plugin data. It is critical
	for merging that this code is called at the start of a plug-in's save and
	load methods. SpecialFX's base implementation saves/loads SpecialFX::name,
	which is used to populate the 'Merge Render Elements' dialog box. if a
	plugin re-implements this function, it should first call
	<b>RenderElement::Save(iSave)</b> or <b>IRenderElement::Load(iLoad)</b>
	\par Parameters:
	<b>ILoad *iLoad</b>\n\n
	You may use this pointer to call methods of ILoad to read data.
	\return  One of the following values: <b>IO_OK</b>, <b>IO_ERROR</b>.
	\par Default Implementation:
	<b>{ return SpecialFX::Load(iLoad); }</b> */
	IOResult Load(ILoad *iLoad)
	{
		return SpecialFX::Load(iLoad);
	}

	/*! \remarks A plug-in which makes references must implement this method
	to receive and respond to messages broadcast by its dependents.
	\par Parameters:
	<b>Interval changeInt</b>\n\n
	This is the interval of time over which the message is active. Currently,
	all plug-ins will receive <b>FOREVER</b> for this interval.\n\n
	<b>RefTargetHandle hTarget</b>\n\n
	This is the handle of the reference target the message was sent by. The
	reference maker uses this handle to know specifically which reference
	target sent the message.\n\n
	<b>PartID\& partID</b>\n\n
	This contains information specific to the message passed in. Some messages
	don't use the <b>partID</b> at all. See \ref Reference_Messages and \ref partids
	for more information about the meaning of the <b>partID</b> for some common messages.\n\n
	<b>RefMessage message</b>\n\n
	The message parameters passed into this method is the specific message
	which needs to be handled. See \ref Reference_Messages.
	\return  The return value from this method is of type <b>RefResult</b>.
	This is usually <b>REF_SUCCEED</b> indicating the message was processed.
	Sometimes, the return value may be <b>REF_STOP</b>. This return value is
	used to stop the message from being propagated to the dependents of the
	item.
	\par Default Implementation:
	<b>{ return REF_SUCCEED; }</b> */
	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// ---------------------
	// from class Animatable
	// ---------------------
	/*! \remarks This method returns the plugin's SuperClass ID.
	\par Default Implementation:
	<b>{ return RENDER_ELEMENT_CLASS_ID; }</b> */
	SClass_ID SuperClassID() { return RENDER_ELEMENT_CLASS_ID; }

	// renderer will call this method to see if IRenderElement is compatible with it
	using SpecialFX::GetInterface;
	/*! \remarks This method is being used as a general extension method in
	3dw max. The Render Elements use it to get the interface for a specific
	renderer's element interface. The renderer will call this method to see if
	an IRenderElement is compatible with it. By asking for a specific renderer
	interface, the element can either provide the interface, or return
	<b>NULL</b>. If <b>NULL</b> is returned, then this element will not be
	available for this renderer. Note that this strategy allows a single render
	element to support more than one renderer. The max default renderer iid is
	in <b>/MAXSDK/INCLUDE/renderElements.h</b> and is defined as
	<b>0xeffeeffe</b>, accessed through <b>MaxRenderElement::IID</b>.
	\par Parameters:
	<b>ULONG id</b>\n\n
	Currently this is not used and is reserved for future use. */
	virtual void* GetInterface(ULONG id) =0;

	/*! \remarks This method is not currently used. It is reserved for future
	use. Its purpose is for releasing an interface created with
	<b>GetInterface()</b>. */
	virtual void ReleaseInterface(ULONG id, void *i) =0;
};

//==============================================================================
// class IRenderElementRequirements
//
// This interface is used to query special requirement flags from render elements.
//==============================================================================
//! \brief This interface is used to query special requirement flags from render elements.
#define IRENDERELEMENTREQUIREMENTS_INTERFACE_ID Interface_ID(0x1804343c, 0x614c2fdf)
class IRenderElementRequirements : public BaseInterface {
public:

	//! \brief Types of requirement
	enum Requirement {
		kRequirement_MotionData = 0 //!< Indicates that the render element requires the renderer to produce motion data
	};

	//! \brief This method is used to know if the renderer has the given requirement.
	//! \param[in] requirement - The requirement to be looked for in the renderer.
	//! \return - true if the renderer has the given requirement, false otherwise.
	virtual bool HasRequirement(Requirement requirement) = 0;

	// -- from BaseInterface
	//! \brief Reimplemented from BaseInterface.
	virtual Interface_ID GetID();
};

inline Interface_ID IRenderElementRequirements::GetID() {

	return IRENDERELEMENTREQUIREMENTS_INTERFACE_ID;
}

//! \brief This returns the IRenderElementRequirements interface for a given render element.
/*!
This method checks for element being NULL and return directly NULL if its the case.
\param[in] element - The render element for which the IRenderElementRequirements interface is wanted.
\return - the IRenderElementRequirements interface for this element or NULL if element is NULL.
*/
inline IRenderElementRequirements* Get_RenderElementRequirements(IRenderElement* element) {

	return (element != NULL) ? static_cast<IRenderElementRequirements*>(element->GetInterface(IRENDERELEMENTREQUIREMENTS_INTERFACE_ID)) : NULL;
}

//////////////////////////////////////////////////////////////
//
//		RenderElement base class for max's default scanline renderer.
//		RenderElement plugins that utilize ShadeContext and IllumParams
//		should sub-class from here
//

/*! \sa  Class IRenderElement, Class ShadeContext, Class IllumParams , <a href="ms-its:3dsmaxsdk.chm::/render_elements.html">Render Elements</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is the RenderElement base class for 3ds Max' default scanline renderer.
RenderElement plugins that utilize ShadeContext and IllumParams should
sub-class from here. . All render elements that support the max renderer should
derive from this class. The class implements a handler for the ShadeOutputIndex
of the elements value in the ShadeOutput array. Access to the elements value is
accomplished within PostIllum or PostAtmosphere by:\n\n
<b>AColor myColor;</b>\n\n
<b>// ... do some computation</b>\n\n
<b>sc.out.elementVals[ mShadeOutputIndex ] = myColor;</b>\n\n
<b>// ... or</b>\n\n
<b>sc.out.elementVals[ ShadeOutputIndex() ] = myColor;</b>\n\n
A sample plugin of a Render Element can be found in the SDK samples;
<b>/MAXSDK/SAMPLES/RENDER/RENDERELEMENTS</b>.
\par Data Members:
protected:\n\n
<b>int mShadeOutputIndex;</b>\n\n
The index into element value array in shadeOutput class.  */
class MaxRenderElement : public IRenderElement
{
	friend class FilterComp;
protected:
	int mShadeOutputIndex;

public:
	// this interface's ID
	enum { IID = 0xeffeeffe };

	// set/get this element's index into the ShadeOutput's array of element channels
	/*! \remarks This method sets the index into the element array in the
	ShadeOutput class. This method is implemented by this base class \& need
	not be re-implemented by derivative classes. Derivative classes access the
	index either through the interface or directly as mShadeOutputIndex. Set is
	used by the system to assign an index to each element. This index persists
	throughout a single rendering.
	\par Parameters:
	<b>int shadeOutputIndex</b>\n\n
	The shadeOutput index. */
	void SetShadeOutputIndex(int shadeOutputIndex) { mShadeOutputIndex = shadeOutputIndex; }
	/*! \remarks This method gets the index into the element array in the
	ShadeOutput class. This method is implemented by this base class \& need
	not be re-implemented by derivative classes. Derivative classes access the
	index either through the interface or directly as mShadeOutputIndex. Set is
	used by the system to assign an index to each element. This index persists
	throughout a single rendering. */
	int ShadeOutputIndex() const { return mShadeOutputIndex; }

	// will be called on each element before call to PostIllum()
	/*! \remarks This method will update the element to time <b>timeValue</b>.
	Note that most elements don't care.\n\n
	Update is called to communicate the time to the element. It will be called
	on each element before the call to <b>PostIllum()</b> is executed. No
	current render element uses this, but it could prove useful someday. This
	class provides a default stub implementation, so this need not be
	implemented unless needed.
	\par Parameters:
	<b>TimeValue timeValue</b>\n\n
	The timevalue at which the update gets called. */
	virtual void Update(TimeValue timeValue) { }

	// compute the element and store in the ShadeContext's ShadeOutput.
	/*! \remarks This method is used to compute the element and store the
	result in the ShadeContext's ShadeOutput.\n\n
	This is the first of the actual computation methods for the render element.
	<b>PostIllum()</b> is called by the material just after an illumination is
	computed for a fragment. Some materials, like the multi-materials don't
	compute illumination themselves but mix illuminations from other leaf
	materials. Such materials do not call <b>PostIllum()</b>, but need to
	consider the elements when blending the shadeoutputs from the leaf
	materials. The ShadeContext is the same shadecontext passed into the
	material's shade method, \& contains the member variable 'out', which is
	the current output from the full pixel shading and the storage place for
	renderElement values.\n\n
	The other param, <b>IllumParams</b>, contains detailed information about
	the shading process. Specifically, the component-wise output from the
	shading process is available in the <b>IllumParams</b>. Also the
	illumParams passed in are dependent on whether the element has requested
	shadows or not. The standard material keeps two sets of these and supplies
	the one requested. This method must be implemented, tho some elements
	merely stub it out or clear the output element value. It should put it's
	output is in <b>sc.out.elementVals[ShadeOutputIndex() ]</b>. Even if you do
	not need this function, it is a good idea to clear the element val.
	\par Parameters:
	<b>ShadeContext\& sc</b>\n\n
	A reference to the ShadeContext.\n\n
	<b>IllumParams\& ip</b>\n\n
	A reference to the IllumParams. */
	virtual void PostIllum(ShadeContext& sc, IllumParams& ip) =0;

	// called after atmospheres are computed, to allow elements to handle atmospheres
	/*! \remarks This is the second computation method and is only called if
	the elements has atmospheres applied. If it is applied, then the element
	value is retrieved from the shadeContext, the atmosphere is applied to it
	\& and the output from the atmosphere is left in <b>sc.out.c</b> and
	<b>sc.out.t</b>. Last, PostAtmosphere is called with the shadeContext, and
	the 2 z values used by the atmosphere to compute it. If the point is
	directly visible to the camera, then <b>prevZ</b> will be 0.0f, otherwise
	it is the z of the next closest obscuring transparent fragment in front of
	the fragment being shaded. It is up to the render element to process the
	output in <b>sc.out.c</b> and save the result in
	<b>sc.out.elementVals[ShadeOutputIndex() ]</b>. Note that when
	PostAtmosphere is called the original value set by PostIllum is saved in
	the element val. This can be overwritten or used in some computation. For
	example, to separate atmosphere from the composite color, <b>sc.out.c</b>
	and <b>sc.out. elementVals[ShadeOutputIndex() ]</b> can be differenced.
	\par Parameters:
	<b>ShadeContext\& sc</b>\n\n
	A reference to the ShadeContext.\n\n
	<b>float z</b>\n\n
	The first depth value.\n\n
	<b>float prevZ</b>\n\n
	The previous depth value. */
	virtual void PostAtmosphere(ShadeContext& sc, float z, float prevZ) =0;

	// ---------------------
	// from class Animatable
	// ---------------------
	// renderer will call this method to see if IRenderElement is compatible with it
	using IRenderElement::GetInterface;
	/*! \remarks The renderer will call this method to see if IRenderElement
	is compatible with it This is used for future expansion in interface
	classes. When the 3ds Max development team needs to add additional
	functionality to an interface class this method provides a way to do that
	without breaking the API. If the 3ds Max developers would add methods to an
	existing class it would invalidate the plug-ins that used the class. Using
	thismethod allows additional functionality to be added to the interface
	class without breaking the API.
	\par Parameters:
	<b>ULONG id</b>\n\n
	Currently this is not used and is reserved for future use. */
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : SpecialFX::GetInterface(id);}
	/*! \remarks This method is not currently used. It is reserved for future
	use. Its purpose is for releasing an interface created with
	<b>GetInterface()</b>. */
	virtual void ReleaseInterface(ULONG id, void *i) { }
};

#pragma warning(pop)
//////////////////////////////////////////////////////////////
//
//		RenderElement base class for texture baking elements using
//		max's default scanline renderer.
//
/*! \sa  Class IRenderElement , Class MaxRenderElement\n\n
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
The class provides the interfaces for creating a Texture Baking plugins in 3ds
Max. The texture baking plugins are controlled through the Maxscript "Render To
Texture". There are methods available in this class that provide the ability to
produce a dynamic UI in Maxscript.\n\n
This class provides the basic infrastructure for the UI and Maxscript
interaction, to fully understand the workings of these plugins please look at
the RenderElement samples\n\n
For an example of this class in use by <b>Texture Baking</b> plug-ins see
<b>/MAXSDK/SAMPLES/RENDER/RENDERELEMENTS/INCLUDE/STDBAKEELEM.CPP</b>\n\n
All methods of this class are virtual.\n\n
   */
class MaxBakeElement : public MaxRenderElement
{

public:
	// this interface's ID
	enum { IID = 0xeffeefff };

	// these are the common attributes for texture bake elements
	// set/get element's output size, shd always be square
	/*! \remarks Set the size of the bitmap to be created\n\n

	\par Parameters:
	<b>int xSz</b>\n\n
	Specifies the width of the output.\n\n
	<b>int ySz</b>\n\n
	Specifies the height of the output.\n\n
	  */
	virtual void SetOutputSz( int xSz, int ySz ) = 0;
	/*! \remarks Get the size of the bitmap to be created\n\n

	\par Parameters:
	<b>int\& xSz</b>\n\n
	The new width\n\n
	<b>int\& ySz</b>\n\n
	the new height\n\n
	  */
	virtual void GetOutputSz( int& xSz, int& ySz ) const = 0;

	/*! \remarks This is the name used in the UI, so it does not consist of
	the path - just the actual file name\n\n

	\par Parameters:
	<b>MCHAR* newName</b>\n\n
	The name to set for the UI\n\n
	  */
	virtual void SetFileName( const MCHAR* newName) = 0;

	// deprecated method - implement SetFileName(const MCHAR *newName) 
	virtual void SetFileName( MCHAR* newName) { SetFileName(const_cast<const MCHAR*>(newName)); }

	/*! \remarks Returns the actual filename. See remark above\n\n
	  */
	virtual const MCHAR* GetFileName() const = 0;
	/*! \remarks This is full name of the file including path, that is used by
	the renderer to actually create the bitmap to store the baked element. This
	should not really be set as it is created dynamically for sole use by the
	renderer.\n\n

	\par Parameters:
	<b>MCHAR* newType</b>\n\n
	The name for the full file name\n\n
	  */
	virtual void SetFileType( const MCHAR* newType) = 0;

	// deprecated method - implement SetFileType(const MCHAR *newType) 
	virtual void SetFileType( MCHAR* newType) { SetFileType(const_cast<const MCHAR*>(newType)); }

	/*! \remarks Returns the filename used for this baked element - See
	comment above\n\n
	  */
	virtual	const MCHAR* GetFileType() const = 0;

	/*! \remarks none  */
	virtual void SetFileNameUnique(BOOL on) = 0; 
	/*! \remarks none  */
	virtual BOOL IsFileNameUnique() const = 0;

	// set/get the renderingbitmap, max of sizes
	// no memory management of the bitmap!
	/*! \remarks The render will create a bitmap from the data available and
	will pass the Bitmap to the plugin. This will be the Bitmap the max
	actually uses to render to.\n\n

	\par Parameters:
	<b>Bitmap* pBitmap</b>\n\n
	The bitmap used for the output\n\n
	  */
	virtual void SetRenderBitmap( Bitmap* pBitmap ) = 0;
	/*! \remarks Returns the bitmap containing the final output\n\n
	  */
	virtual Bitmap* GetRenderBitmap() const = 0;

	// set/get whether the output texture is shaded, or represents raw
	// texture colors
	/*! \remarks This defines whether the baker uses lighting or not. For
	example a diffuse texture baker can specify whether the result uses
	lighting or not.\n\n

	\par Parameters:
	<b>BOOL on</b>\n\n
	The state for the lighting parameter\n\n
	  */
	virtual void SetLightApplied(BOOL on) = 0; 
	/*! \remarks Returns whether lighting is used in the calculation\n\n
	  */
	virtual BOOL IsLightApplied() const = 0;

	// set/get element's shadow applied state
	/*!   \remarks This defines whether the baker uses shadows or not.
	For example a diffuse texture bake can specify whether the result uses
	shadows or not.\n\n

	\par Parameters:
	<b>BOOL on</b>\n\n
	The state for the shadow parameter\n\n
	  */
	virtual void SetShadowApplied(BOOL on) = 0;
	/*!   \remarks Returns whether shadows are used in the
	calculation\n\n
	  */
	virtual BOOL IsShadowApplied() const = 0;


	// set/get element's atmosphere applied state
	/*!   \remarks Defines whether the texture baking uses Atmosphere in
	the final result.\n\n

	\par Parameters:
	<b>BOOL on</b>\n\n
	Specifies whether Atmospheres are used or not.\n\n
	  */
	virtual void SetAtmosphereApplied(BOOL on) = 0;
	/*! \remarks Returns whether Atmospheres are used. Most texture baking
	plugins would return false.\n\n
	  */
	virtual BOOL IsAtmosphereApplied() const = 0;

		// these IRenderElement methods hooked to new functions
	virtual	BOOL AtmosphereApplied() const {
			return IsAtmosphereApplied();
		}
	virtual	BOOL ShadowsApplied() const {
			return IsShadowApplied();
		}

	// these allow us to build dynamic ui for the elements in maxscript
	// currently only checkboxes supported
	// these use & return 0-based indices
	/*! \remarks Return the number of parameters to be exposed to the UI\n\n
	  */
	virtual int GetNParams() const = 0;
	/*! \remarks Returns the name used by maxscript for the parameter value
	passed in. This is a 1 based access, so if GetNParams returned 8 the valid
	range for GetParamName will be 1 to 8.\n\n

	\par Parameters:
	<b>int nParam</b>\n\n
	The value for the parameter whose name is to be returned.\n\n
	  */
	virtual	const MCHAR* GetParamName( int nParam ) = 0;
	/*! \remarks Returns the index of the parameter based on the name\n\n
	\code
	const int FindParamByName1( MCHAR* name )
	{
		for( int i = 0; i < 8; ++i ) {
			if( strcmp( name, GetString( paramNames[i] )) == 0
				)
				return i+1;
		}
		return 0;
	}
	\endcode */
	virtual	const int FindParamByName( MCHAR* name ) = 0;
	/*! \remarks Currently only 1 = Boolean, 0 = Undefined are supported. In
	release 5 this means that any parameter you have is defined and used as a
	Boolean in maxscript\n\n

	\par Parameters:
	<b>int nParam</b>\n\n
	The value for the parameter whose type is to be returned.\n\n
	  */
	virtual	int  GetParamType( int nParam ) = 0;
	/*! \remarks The value to set when the Render to Texture tool is
	loaded\n\n

	\par Parameters:
	<b>int nParam</b>\n\n
	The value for the parameter whose value is to be returned.\n\n
	  */
	virtual	int  GetParamValue( int nParam ) = 0;
	/*! \remarks The Render to Texture Tool will save out the data
	\par Parameters:
	<b>int nParam</b>\n\n
	The value for the parameter whose value is to be set\n\n
	<b>int newVal</b>\n\n
	The value to set */
	virtual	void SetParamValue( int nParam, int newVal ) = 0;

	// ---------------------
	// from class Animatable
	// ---------------------
	SClass_ID SuperClassID() { return BAKE_ELEMENT_CLASS_ID; }
	
	// renderer will call this method to see if IRenderElement is compatible with it
	using MaxRenderElement::GetInterface;
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : MaxRenderElement::GetInterface(id);}
	
	virtual void ReleaseInterface(ULONG id, void *i) { UNUSED_PARAM(id); UNUSED_PARAM(i); }
};

//////////////////////////////////////////////////////////////
//
//! \brief Extention to MaxBakeElement in 3ds Max 8
/*! Extends MaxBakeElement, allowing Texture Bake Elements to provide parameters
    of integer, float, and color types in addition to booleans.
    The RTT dialog is updated to display spinners and color swatches accordingly */
class MaxBakeElement8 : public MaxBakeElement
{
public:
	//! \brief MaxBakeElement8 interface ID
	enum { IID = 0x1f2203c5 };

	//! \brief Allows for getting the bake element parameter types
	/*! \param[in] nParam Parameter index, 0-based
	    \return Parameter type, 1 for TYPE_BOOL, 2 for TYPE_INT, 3 for TYPE_FLOAT, 4 for TYPE_FRGBA_BV, or 0 (TYPE_VOID) on error */
	virtual	int GetParamType( int nParam ) = 0;

	//! \brief Allows for getting the bake element parameters as seen in the RTT dialog
	/*! \param[in] nParam Parameter index, 0-based
	    \return Parameter value of TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_FRGBA_BV, or TYPE_VOID on error */
	virtual	FPValue GetParamFPValue( int nParam ) = 0;

	//! \brief Allows for settings the bake element parameters as seen in the RTT dialog
	/*! \param[in] nParam Parameter index, 0-based
	    \param[in] newVal Parameter value of TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_FRGBA_BV */
	virtual	void SetParamFPValue( int nParam, FPValue newVal ) = 0;


	//! \brief Allows the bake element to specify a minimum value for int and float parameters
	/*! \param[in] nParam The parameter index, 0-based
	    \return Minimum parameter value, of TYPE_INT or TYPE_FLOAT */
	virtual	FPValue GetParamFPValueMin( int nParam ) = 0;

	//! \brief Allows the bake element to specify a maximum value for int and float parameters
	/*! \param[in] nParam The parameter index, 0-based
	    \return Maximum parameter value, of TYPE_INT or TYPE_FLOAT */
	virtual	FPValue GetParamFPValueMax( int nParam ) = 0;

	// ---------------------
	// from class Animatable
	// ---------------------
	using MaxBakeElement::GetInterface;
	//! \brief Renderer calls this method to see if IRenderElement is compatible with it
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : MaxBakeElement::GetInterface(id);}
	
};

//////////////////////////////////////////////////////////////
//
//! \brief Extention to MaxBakeElement8 in 3ds Max 10
/*! Extends MaxBakeElement8, allowing Texture Bake Elements to provide a bitmap background color.*/
class MaxBakeElement10 : public MaxBakeElement8
{
public:
	//! \brief MaxBakeElement10 interface ID
	enum { IID = 0x3b6566df };

	//! \brief Get the background color of the bitmap to be created
	/*! \return The background color */
	virtual	AColor GetBackgroundColor( ) = 0;

	//! \brief Set the background color of the bitmap to be created
	/*! \param[in] color The background color */
	virtual	void SetBackgroundColor( AColor color ) = 0;

	// ---------------------
	// from class Animatable
	// ---------------------
	using MaxBakeElement::GetInterface;
	//! \brief Renderer calls this method to see if IRenderElement is compatible with it
	virtual void* GetInterface(ULONG id){ return (id == IID) ? this : MaxBakeElement8::GetInterface(id);}

};

//////////////////////////////////////////////////////////////
//
//		IRenderElement compatibility interface
//
//		The system will ask a Renderer for this interface by calling
//		Renderer::GetInterface(IRenderElementCompatible::ICompatibleID).
//		If the Renderer returns a pointer to this interface, it is implied that the Renderer
//		supports render elements. The system will then call IRenderElementCompatible::IsCompatible()
//		to determine if an IRenderElement instance is compatible with it.
//
//		To determine compatibility, the Renderer can call IRenderElement::GetInterface()
//		(inherited from class Animatable), passing in an interface ID that a compatible IRenderElement
//		would understand. If the Renderer receives a valid interface pointer, it can return TRUE to
//		IRenderElementCompatible::IsCompatible().
//
//		for an example, see class IScanRenderElement below
//
/*! \sa  Class IRenderElement , <a href="ms-its:3dsmaxsdk.chm::/render_elements.html">Render Elements</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
The system will ask a Renderer for this interface by calling
<b>Renderer::GetInterface (IRenderElementCompatible::IID)</b>. If the Renderer
returns a pointer to this interface, it is implied that the Renderer supports
render elements. The system will then call
<b>IRenderElementCompatible::IsCompatible()</b> to determine if an
IRenderElement instance is compatible with it. To determine compatibility, the
Renderer can call <b>IRenderElement::GetInterface()</b> (inherited from class
Animatable), passing in an interface ID that a compatible IRenderElement would
understand. If the Renderer receives a valid interface pointer, it can return
TRUE to <b>IRenderElementCompatible::IsCompatible()</b>.  */
class IRenderElementCompatible: public MaxHeapOperators
{
public:
	// this interface's ID
	enum { IID = 0xcafeface };

	/*! \remarks This method determines if the specified IRenderElement
	instance is compatible with the renderer.
	\par Parameters:
	<b>IRenderElement *pIRenderElement</b>\n\n
	A pointer to the IRenderElement to check for compatibility.
	\return  TRUE if the RenderElement is compatible, FALSE if it is not. */
	virtual BOOL IsCompatible(IRenderElement *pIRenderElement) = 0;
};


//////////////////////////////////////////////////////////////
//
//		RenderElementMgr base interface
//
#define IREND_ELEM_MGR_INTERFACE Interface_ID(0x95791767, 0x17651746)


/*! \sa  Class IRenderElement, Class FPMixinInterface, Class ReferenceTarget, <a href="ms-its:3dsmaxsdk.chm::/render_elements.html">Render Elements</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface for the Render Element Manager. A sample
plugin of a Render Element can be found in the SDK samples;
<b>/MAXSDK/SAMPLES/RENDER/RENDERELEMENTS</b>.  */
class IRenderElementMgr : public FPMixinInterface
{
public:

	// called by system to add an element merged from another file
	/*! \remarks This method gets called by the system to add a Render Element
	when it is merged from another file.
	\par Parameters:
	<b>IRenderElement *pRenderElement</b>\n\n
	A pointer to the Render Element to add.
	\return  TRUE if the merging was successful, FALSE if it was not. */
	virtual BOOL AppendMergedRenderElement(IRenderElement *pRenderElement) = 0;
	/*! \remarks This method gets called by the system to add a Render Element
	when it is merged from another file and ensures that the Reference Target
	is a Render Element.
	\par Parameters:
	<b>ReferenceTarget *pRenderElement</b>\n\n
	A pointer to the Render Element to add.
	\return  TRUE if the merging was successful, FALSE if it was not. */
	virtual BOOL AppendMergedRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element

	// adds/removes an instance of IRenderElement to the manager's list
	/*! \remarks This method adds an IRenderElement instance to the manager's
	list.
	\par Parameters:
	<b>IRenderElement *pRenderElement</b>\n\n
	A pointer to the Render Element to add.
	\return  TRUE if the addition was successful, FALSE if it was not. */
	virtual BOOL AddRenderElement(IRenderElement *pRenderElement) = 0;
	/*! \remarks This method adds an IRenderElement instance to the manager's
	list and ensures that the Reference Target is a Render Element.
	\par Parameters:
	<b>ReferenceTarget *pRenderElement</b>\n\n
	A pointer to the Render Element to add.
	\return  TRUE if the addition was successful, FALSE if it was not. */
	virtual BOOL AddRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	virtual BOOL RemoveRenderElement(IRenderElement *pRenderElement) = 0;
	/*! \remarks This method removes an IRenderElement instance from the
	manager's list and ensures that the Reference Target is a Render Element.
	\par Parameters:
	<b>ReferenceTarget *pRenderElement</b>\n\n
	A pointer to the Render Element to remove.
	\return  TRUE if the addition was successful, FALSE if it was not. */
	virtual BOOL RemoveRenderElement(ReferenceTarget *pRenderElement) = 0; // ensures ReferenceTarget is a render element
	/*! \remarks This method will remove all the IRenderElement instances from
	the manager's list. */
	virtual void RemoveAllRenderElements() = 0;

	// returns number of render elements in manager's list
	/*! \remarks This method returns the number of Render Elements contained
	in the manager's list. */
	virtual int NumRenderElements() = 0;

	// returns pointer to a specific render element in manager's list -- NULL if invalid index
	/*! \remarks This method returns a pointer to a specific Render Element in
	manager's list.
	\par Parameters:
	<b>int index</b>\n\n
	The index of the Render Element in the manager's list.
	\return  A pointer to the specific Render Element or NULL if the index is
	invalid. */
	virtual IRenderElement *GetRenderElement(int index) = 0;

	// sets/gets whether element list should be active during a render
	/*! \remarks This method sets whether the Render Elements List should be
	active during a render.
	\par Parameters:
	<b>BOOL elementsActive</b>\n\n
	TRUE to activate the Render Elements List, FALSE to deactivate. */
	virtual void SetElementsActive(BOOL elementsActive) = 0;
	/*! \remarks This method checks if the Render Elements List is active
	during a render and will return TRUE if it is or FALSE if it is not. */
	virtual BOOL GetElementsActive() const = 0;

	// sets/gets whether elements should be displayed in their own viewer
	/*! \remarks This method sets whether the Render Elements should be
	displayed in their own (VFB) viewer window.
	\par Parameters:
	<b>BOOL displayElements</b>\n\n
	TRUE to display in their own viewer, FALSE if you do not want to. */
	virtual void SetDisplayElements(BOOL displayElements) = 0;
	/*! \remarks This method checks whether the Render Elements are displayed
	in their own (VFB) viewer window and will return TRUE if they are or FALSE
	if they are not. */
	virtual BOOL GetDisplayElements() const = 0;

	// sets/gets whether element list should be exported to Combustion file format
	/*! \remarks This method sets whether the Render Element List should be
	exported to a Combustion format file.
	\par Parameters:
	<b>BOOL combustionOutEnabled</b>\n\n
	TRUE to enable Combustion format file output. */
	virtual void SetCombustionOutputEnabled(BOOL combustionOutEnabled) = 0;
	/*! \remarks This method checks whether the Render Element List will be
	exported to a Combustion format file and will return TRUE if they are or
	FALSE if they are not. */
	virtual BOOL GetCombustionOutputEnabled() const = 0;

	/*! \remarks This method allows you to set the output Asset for a
	Combustion format output file.
	\par Parameters:
	<b>const MaxSDK::AssetManagement::AssetUser& combustionOutputPath</b>\n\n
	The AssetUser. */
	virtual void SetCombustionOutput(const MaxSDK::AssetManagement::AssetUser& combustionOutputPath) = 0;
	/*! \remarks This method returns the output Asset for a Combustion format
	file. */
	virtual const MaxSDK::AssetManagement::AssetUser& GetCombustionOutput() const = 0;


#ifndef NO_RENDER_ELEMENTS
	// function publishing
	enum
	{
		fps_AddRenderElement, fps_RemoveRenderElement, fps_RemoveAllRenderElements,
		fps_NumRenderElements, fps_GetRenderElement,
		fps_SetElementsActive, fps_GetElementsActive,
		fps_SetDisplayElements,	fps_GetDisplayElements,
		fps_SetCombustionOutputEnabled, fps_GetCombustionOutputEnabled,
		fps_SetCombustionOutputPath, fps_GetCombustionOutputPath,
		fps_SetRenderElementFilename, fps_GetRenderElementFilename,
	};
#endif // NO_RENDER_ELEMENTS

};

