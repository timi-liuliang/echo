/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

   FILE: sfx.h

	 DESCRIPTION: class declarations for the special effect class hierarchy

	 CREATED BY: michael malone (mjm)

	 HISTORY: created June 30, 2000

   	 Copyright (c) 2000, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */
#pragma once

#include "iFnPub.h"
#include "plugapi.h"
#include "GetCOREInterface.h"
#include "ref.h"

// forward declarations
class Bitmap;
class IRendParams;
class ShadeContext;
class RenderGlobalContext;
class IReshadeFragment;

#pragma warning(push)
#pragma warning(disable:4100)


// ------------------------------
//
// check abort callback class
//
// used to check for abort during
// an extended operation
// and to update progress
//
// ------------------------------

#define CAC_INTERFACE Interface_ID(0x666534, 0x50101) 

/*! \sa  Class Effect.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
Implemented by the System (for Render Effect plug-ins inside the
<b>Effect::Apply()</b> method).\n\n
The <b>Check()</b> method of this class may be called to check if the user did
something to abort the application of the effect.  */
class CheckAbortCallback : public FPMixinInterface
{
public:
	/*! \remarks Returns TRUE if user has done something to cause an abort; otherwise FALSE. */
	virtual BOOL Check()=0;  // returns TRUE if user has done something to cause an abort.
	/*! \remarks This method is should be called by each Effect plug-in as it
	proceeds through the image to update the progress bar.
	\par Parameters:
	<b>int done</b>\n\n
	The amount done, i.e. the current state of the image processsing. This is
	usually the number of scan lines processed so far.\n\n
	<b>int total</b>\n\n
	The total number of updates. This is usually the number of pixels in height of
	the image.
	\return  Returns TRUE if user has done something to cause an abort; otherwise	FALSE. */
	virtual	BOOL Progress(int done, int total)=0;
	/*! \remarks This method is called internally by the calling code -- plug-ins
	don't need to call this method. */
	virtual void SetTitle(const MCHAR *title)=0;

    // FPInterface stuff
	enum { cac_check, cac_progress, cac_setTitle, };

	BEGIN_FUNCTION_MAP	
      FN_0(cac_check, TYPE_BOOL, Check); 
      FN_2(cac_progress, TYPE_BOOL, Progress, TYPE_INT, TYPE_INT); 
      VFN_1(cac_setTitle, SetTitle, TYPE_STRING); 
    END_FUNCTION_MAP 

	FPInterfaceDesc* GetDesc() { return (FPInterfaceDesc*)GetCOREInterface(CAC_INTERFACE); }
};

// -------------------------------------------------------------
//
// special effect plug-in base class
//
// base class for Atmospherics, Effects, RenderElements, Shaders
//
// -------------------------------------------------------------

/*! \sa  Class Effect, Class Atmospheric, Class Sampler.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
An instance of this class is returned by a rendering effect, atmopsheric
plug-in, or sampler when it is asked to put up its rollup page in the user
interface.  */
class SFXParamDlg : public InterfaceServer
{
public:
	/*! \remarks Returns the unique Class_ID of this item. */
	virtual Class_ID ClassID()=0;
	/*! \remarks This sets the current filter, sampler, atmospheric or rendering effect
	being edited to the one passed and updates the user interface controls to
	reflect the state of the new 'thing'.
	\par Parameters:
	<b>ReferenceTarget *m</b>\n\n
	The effect to save as current. */
	virtual void SetThing(ReferenceTarget *m)=0;
	/*! \remarks This returns the current filter, sampler, atmospheric or rendering effect
	being edited. */
	virtual ReferenceTarget* GetThing()=0;
	/*! \remarks This method is called when the current time has changed. This gives the
	developer an opportunity to update any user interface data that may need
	adjusting due to the change in time.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The new current time.
	\par Default Implementation:
	<b>{}</b> */
	virtual void SetTime(TimeValue t) { }		
	/*! \remarks Deletes this instance of the class. */
	virtual void DeleteThis()=0;		
	/*! \remarks This is a general purpose function that allows the Max SDK to be
	extended in the future. The 3ds Max development team can assign new command numbers
	and continue to add functionality to this class without having to 'break' the	Max SDK.
	\par Parameters:
	<b>int cmd</b>\n\n
	The index of the command to execute.\n\n
	<b>ULONG_PTR arg1=0</b>\n\n
	Optional argument 1. See the documentation where the cmd option is discussed
	for more details on these parameters.\n\n
	<b>ULONG_PTR arg2=0</b>\n\n
	Optional argument 2.\n\n
	<b>ULONG_PTR arg3=0</b>\n\n
	Optional argument 3.
	\return  An integer return value. See the documentation where the <b>cmd</b>
	option is discussed for more details on the meaning of this value.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; } 
	virtual void InvalidateUI() { }	
};


/*! \sa  Class ReferenceTarget, Class SFXParamDlg, Class IRendParams, Class Atmospheric, Class Effect, Class AppendGizmoRestore, Class DeleteGizmoRestore.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class for Atmospheric, Renderer Effect, and Shader Plug-Ins.
It contains a few methods common to each of those plug-in classes.
\par Data Members:
<b>MSTR name;</b>\n\n
This is the name which appears in Track %View.  */
class SpecialFX: public ReferenceTarget
{
public:
	MSTR name;
	// This name will appear in the track view and the list of current atmospheric effects.
	/*! \remarks This method is used to retrieve the name for the plug-in. This name will
	appear in the track view and the list of current atmospheric or rendering
	effects. */
	virtual MSTR GetName() { return _M(""); }		

	// Is effect active 
	/*! \remarks Returns TRUE if the effect is active; otherwise FALSE.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to check.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL Active(TimeValue t) { return !TestAFlag(A_ATMOS_DISABLED); }

	// Called when the render steps to a new frame
	/*! \remarks This method is called once per frame when the renderer begins. This gives
	the atmospheric or rendering effect the chance to cache any values it uses
	internally so they don't have to be computed on every frame.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The current time of the call.\n\n
	<b>Interval\& valid</b>\n\n
	The validity interval of the cache created by the plug-in. The plug-in may
	set this for its own use. The plug-in can then check if the cache is up to
	date and update it if not. */
	virtual	void Update(TimeValue t, Interval& valid) { }

	// Saves and loads name. These should be called at the start of
	// a plug-in's save and load methods.
	/*! \remarks Implemented by the System.\n\n
	To facilitate naming atmospheric or rendering effects, a 'name' string has
	been added to the base class. This method should be called from the
	developers sub-classed Atmospheric or Effects plug-in to save the name. */
	CoreExport IOResult Save(ISave *isave);
	/*! \remarks Implemented by the System.\n\n
	To facilitate naming atmospheric or rendering effects, a 'name' string has
	been added to the base class. This method should be called from the
	developers sub-classed Atmospheric or Effects plug-in to load the name. */
	CoreExport IOResult Load(ILoad *iload);

	// Put up a dialog that lets the user edit the plug-ins parameters.
	/*! \remarks	This method creates and returns a new instance of a class derived from
	<b>SFXParamDlg</b> to manage the user interface. This put up a modal dialog
	that lets the user edit the plug-ins parameters.
	\par Parameters:
	<b>IRendParams *ip</b>\n\n
	This is the interface given to the plug-in so it may display its
	parameters.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual SFXParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }
	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// effect has secondary dialogs that don't have the effect as their 'thing'.
	// Called once for each secondary dialog for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	/*! \remarks	Implement this if you are using the ParamMap2 AUTO_UI system and the effect
	has secondary dialogs that don't have the effect as their 'thing'. Called
	once for each secondary dialog for you to install the correct thing.\n\n
	Note: Developers needing more information on this method can see the
	remarks for
	<a href="class_mtl_base.html#A_GM_mtlbase_ui">MtlBase::CreateParamDlg()</a>
	which describes a similar example of this method in use (in that case it's
	for use by a texture map plug-in).
	\par Parameters:
	<b>SFXParamDlg* dlg</b>\n\n
	Points to the ParamDlg.
	\return  Return TRUE if you process the dialog; otherwise FALSE.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL SetDlgThing(SFXParamDlg* dlg) { return FALSE; }

	// If an atmospheric has references to gizmos or other objects in the scene it can optionally 
	// provide access to the object list.
	/*! \remarks If an atmospheric or rendering effect has references to gizmos or other
	objects in the scene it can optionally provide access to the object list.
	This method returns the number of gizmos or objects the plug-in has.
	\par Default Implementation:
	<b>{return 0;}</b> */
	virtual int NumGizmos() { return 0; }
	/*! \remarks Returns a pointer to the 'i-th' gizmo or object node.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the gizmo to return.
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual INode *GetGizmo(int i) { return NULL; }
	/*! \remarks Deletes the 'i-th' gizmo or object from those used by the plug-in.
	\par Parameters:
	<b>int i</b>\n\n
	The index of the gizmo to delete.
	\par Default Implementation:
	<b>{}</b> */
	virtual void DeleteGizmo(int i) { }
	/*! \remarks Adds the specified node to the end of the list of gizmos used by the
	plug-in.
	\par Parameters:
	<b>INode *node</b>\n\n
	Points to the node to append.
	\par Default Implementation:
	<b>{}</b> */
	virtual void AppendGizmo(INode *node) { }
	/*! \remarks This method is called to approve a node for possible use as gizmo. Return
	TRUE if the node is okay; otherwise FALSE.
	\par Parameters:
	<b>INode *node</b>\n\n
	The node to check.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL OKGizmo(INode *node) { return FALSE; } // approve a node for possible use as gizmo
	/*! \remarks This method is called to select the specified gizmo and displays parameters
	for it (if any).\n\n
	In the Special Effects section of the light dialog there is a button
	labelled 'Setup'. The Setup button brings up the Environment dialog (for
	Atmospherics) or the Render Effects dialog (for Render Effects) and selects
	the choosen effect. It also selects the "gizmo" within that effect, so if
	there are particular parameters for each gizmo the user will see them.
	Pressing that button causes this method to be called.
	\par Parameters:
	<b>INode *node</b>\n\n
	The gizmo node.
	\par Default Implementation:
	<b>{}</b> */
	virtual void EditGizmo(INode *node) { } // selects this gizmo & displays params for it if any
	/*! \remarks Inserts the specified gizmo node into the list of gizmos. This method must
	be defined to use the <b>DeleteGizmoRestore</b> class.
	\par Parameters:
	<b>int i</b>\n\n
	The zero based index of the position in the list of where the insertion
	should take place.\n\n
	<b>INode *node</b>\n\n
	The gizmo node to insert.
	\par Default Implementation:
	<b>{ assert(0); }</b> */
	virtual	void InsertGizmo(int i, INode *node) { assert(0); } // must be defined to use DeleteGizmoRestore

	// Animatable overides...
	CoreExport SvGraphNodeReference SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags);
};

#define SFXBASE_CHUNK	0x39bf
#define SFXNAME_CHUNK	0x0100


// Classes used for implementing UNDO in Atmosphere and Effects classes.
/*! \sa  Class RestoreObj, Class Atmospheric, Class Effect, Class SpecialFX, Class INode.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class enables implementing undo of Gizmo assignment in Atmosphere and
Effects classes. This class provides implementations of the <b>RestoreObj</b>
methods. An instance of this class can be put on the Hold stack when a Gizmo is
appended. For example:
\code
if (theHold.Holding())
	theHold.Put(new AppendGizmoRestore(this,node));
\endcode  
All methods of this class are implemented by the System.
\par Data Members:
<b>SpecialFX *fx;</b>\n\n
Points to the Atmosphere of Effect.\n\n
<b>INode *node;</b>\n\n
Points to the gizmo node.  */
class AppendGizmoRestore : public RestoreObj
{
public:
	SpecialFX *fx;
	INode *node;
	/*! \remarks Constructor. The data members are initalized to the values
	pased. */
	AppendGizmoRestore(SpecialFX *f, INode *n) { fx= f; node = n; }
	void Redo() { fx->AppendGizmo(node); }
	void Restore(int isUndo) { fx->DeleteGizmo(fx->NumGizmos()-1); } 
	MSTR Description() { return MSTR("AppendGizmoRestore"); }
};

/*! \sa  Class RestoreObj, Class Atmospheric, Class Effect, Class SpecialFX, Class INode.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class enables implementing undo of Gizmo deletion in Atmosphere and
Effects classes. This class provides implementations of the <b>RestoreObj</b>
methods. An instance of this class can be put on the Hold when a Gizmo is
deleted. For example:
\code
if (theHold.Holding())
	theHold.Put(new DeleteGizmoRestore(this,nodes[i],i));
\endcode  
All methods of this class are implemented by the System.
\par Data Members:
<b>SpecialFX *fx;</b>\n\n
Points to the Atmosphere of Effect.\n\n
<b>INode *node;</b>\n\n
Points to the gizmo node.\n\n
<b>int num;</b>\n\n
The index of the gizmo which is being deleted.  */
class DeleteGizmoRestore: public RestoreObj
{
public:
	SpecialFX *fx;
	INode *node;
	int num;
	/*! \remarks Constructor. The data members are initialized to the
	parameter passed. */
	DeleteGizmoRestore(SpecialFX *a, INode *n, int i) {	fx = a; node = n; num = i;	}
	void Redo() { fx->DeleteGizmo(num); }
	void Restore(int isUndo) { fx->InsertGizmo(num,node); } 
	MSTR Description() { return MSTR("DeleteGizmoRestore"); }
};

// ------------------------------
// atmospheric plug-in base class
// ------------------------------

// Returned by an Atmospheric when it is asked to put up its rollup page.
typedef SFXParamDlg AtmosParamDlg;

/*! \sa  Class SpecialFX, Class SFXParamDlg, Class IRendParams, Class ShadeContext, Class Point3,  Class Color,  Class Interval.\n\n
\par Description:
This is the base class for the creation of Atmospheric plug-ins. Developers may
look to the text below for information on the techniques used to create these
types of effects:\n\n
Kenton Musgrave, Darwyn Peachey, Ken Perlin, Steven Worley, Texturing and
Modeling A Procedural Approach (Cambridge, MA: Academic Press, Inc., 1994).
ISBN: 0-12-228760-6.\n\n
Chapter 6 on Hypertexture, particularly the section on 'Raymarching', describes
essentially the method 3ds Max uses.  */
class Atmospheric : public SpecialFX
{
public:
	RefResult NotifyRefChanged(Interval changeInt,
		                       RefTargetHandle hTarget,
							   PartID& partID,
							   RefMessage message) { return REF_SUCCEED; }
	SClass_ID SuperClassID() { return ATMOSPHERIC_CLASS_ID; }
	
	// Saves and loads name. These should be called at the start of
	// a plug-in's save and load methods.
	/*! \remarks	Implemented by the system.\n\n
	To facilitate naming atmospheric effects, a 'name' string has been added to
	the base class. This method should be called from the developers
	sub-classed Atmospheric plug-in to save the name. */
	IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
	/*! \remarks	Implemented by the system.\n\n
	To facilitate naming atmospheric effects, a 'name' string has been added to
	the base class. This method should be called from the developers
	sub-classed Atmospheric plug-in to load the name. */
	IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

	// Put up a modal dialog that lets the user edit the plug-ins parameters.
	/*! \remarks This method creates and returns a new instance of a class derived from
	<b>AtmosParamDlg</b> to manage the user interface. This put up a modal
	dialog that lets the user edit the plug-ins parameters.
	\par Parameters:
	<b>IRendParams *ip</b>\n\n
	This is the interface given to the atmospheric effect so it may display its
	parameters.
	\return  A new instance of a class derived from <b>AtmosParamDlg</b>.\n\n
	Note: <b>typedef SFXParamDlg AtmosParamDlg;</b>
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual AtmosParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }
	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// atmosphere has secondary dialogs that don't have the effect as their 'thing'.
	// Called once for each secondary dialog for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	/*! \remarks	You should implement this if you are using the ParamMap2 AUTO_UI system and
	the effect has secondary dialogs that have something other than the
	incoming effect as their 'thing'. Called once for each secondary dialog for
	you to install the correct thing. Return TRUE if you process the dialog,
	FALSE otherwise, in which case the incoming effect will be set into the
	dialog.\n\n
	Note: Developers needing more information on this method can see the
	remarks for
	<a href="class_mtl_base.html#A_GM_mtlbase_ui">MtlBase::CreateParamDlg()</a>
	which describes a similar example of this method in use (in that case it's
	for use by a texture map plug-in).
	\par Parameters:
	<b>AtmosParamDlg* dlg</b>\n\n
	Points to the ParamDlg.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL SetDlgThing(AtmosParamDlg* dlg) { return FALSE; }

	// This is the function that is called to apply the effect.
	/*! \remarks This is the function that is called to apply the atmospheric effect. The
	line segment defined by the world space points <b>p0</b> and <b>p1</b>
	represent a segment of volume that needs to be shaded. This line segment is
	a line along the ray defined by the line between the viewer's eye (the
	camera) and the pixel being rendered in the image plane and continuing
	through world space. This ray is broken up into segments, with the
	boundaries defined by surfaces. If there are no surfaces, there will just
	be a single segment from the eye point going off into space (<b>p1</b> will
	be a large number). If there is a surface that is hit, and the surface is
	opaque, there will still be one segment from the eye to the surface. If the
	surface is transparent there may be two segments, the segment before and
	the segment after. Therefore the ray may be broken up into many segments
	depending on the number of transparent surfaces hit.\n\n
	The shader does not need to be directly concerned with this however. It
	only knows it's shading the segment between <b>p0</b> and <b>p1</b>. It
	will get called repeatedly for different pixels and different segments.\n\n
	The output of this method is the computed <b>color</b> and
	<b>trans</b>parency.\n\n
	An example implementation of this method is 3ds Max's Simple Fog. It takes
	the distance of the line segment and essentially interpolates towards the
	fog color based on the distance. This is a very simple effect.\n\n
	3ds Max's Volume Fog traverses along the segment and evaluates its 3D noise
	function. It integrates the density across the segment, and uses the
	density to compute the fog color.
	\par Parameters:
	<b>ShadeContext\& sc</b>\n\n
	The ShadeContext.\n\n
	<b>const Point3\& p0</b>\n\n
	The start point of the segment to shade. This point (and <b>p1</b>) are in
	an undefined 'internal' space specific to the renderer (which for the 3ds
	Max renderer is in fact is camera space). To get to world space the plug-in
	would call <b>sc.PointTo(p0,REF_WORLD)</b>.\n\n
	<b>const Point3\& p1</b>\n\n
	The end point of the segment to shade.\n\n
	<b>Color\& color</b>\n\n
	This method shades the volume between p0 and p1 and modifies this
	color.\n\n
	<b>Color\& trans</b>\n\n
	This method shades the volume between p0 and p1 and modifies this
	transparency.\n\n
	<b>BOOL isBG=FALSE</b>\n\n
	TRUE if the background is being shaded; otherwise FALSE. If TRUE then p1
	will be infinity (a large number). This is used when the option to not fog
	the background is on. */
	virtual void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE)=0;
// begin - ke/mjm - 03.16.00 - merge reshading code
//	virtual void PreShade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, IReshadeFragment* pFrag, BOOL isBG=FALSE) { }
//	virtual void ReShade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, IReshadeFragment* pFrag, BOOL isBG=FALSE) { Shade(sc,p0,p1,color,trans,isBG); }
// end - ke/mjm - 03.16.00 - merge reshading code
};

// Chunk IDs saved by base class
#define ATMOSHPERICBASE_CHUNK	SFXBASE_CHUNK
#define ATMOSHPERICNAME_CHUNK	SFXNAME_CHUNK


// --------------------------------
// render effect plug-in base class
// --------------------------------

// Returned by an  effect when it is asked to put up its rollup page.
typedef SFXParamDlg EffectParamDlg;

/*! \sa  Class SpecialFX, Class SFXParamDlg, Class IRendParams, Class RenderGlobalContext,  Class Bitmap, Class ISave,  Class ILoad,  Class INode, Class Interface.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the base class used in the creation of Rendering Effect plug-ins. In
3ds Max 3.0 these plug-in are added in sequence after a rendering is complete
without the use of Video Post. A developer creates a sub-class of this class
and implements or calls the methods shown below.\n\n
<b>Plug-In Information:</b>\n\n
Class Defined In RENDER.H\n\n
Super Class ID RENDER_EFFECT_CLASS_ID\n\n
Standard File Name Extension DLV\n\n
Extra Include File Needed None\n\n
There are also methods in the Interface class for manipulating the Effects
List:\n\n
<b>virtual int NumEffects()=0;</b>\n\n
<b>virtual Effect *GetEffect(int i)=0;</b>\n\n
<b>virtual void SetEffect(int i,Effect *e)=0;</b>\n\n
<b>virtual void AddEffect(Effect *eff)=0;</b>\n\n
<b>virtual void DeleteEffect(int i)=0;</b>  */
class Effect : public SpecialFX
{
public:
	RefResult NotifyRefChanged(Interval changeInt,
		                       RefTargetHandle hTarget,
							   PartID& partID,
							   RefMessage message) {return REF_SUCCEED;}
	SClass_ID SuperClassID() { return RENDER_EFFECT_CLASS_ID; }

	// Saves and loads name. These should be called at the start of
	// a plug-in's save and load methods.
	/*! \remarks Implemented by the System.\n\n
	To facilitate naming rendering effects a 'name' string exists in the base
	class. This method should be called at the start of the developer's
	sub-classed Effect plug-in to save the name.
	\par Parameters:
	<b>ISave *isave</b>\n\n
	An interface for saving data. */
	IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
	/*! \remarks Implemented by the System.\n\n
	To facilitate naming rendering effects a 'name' string exists in the base
	class. This method should be called at the start of the developer's
	sub-classed Effect plug-in to load the name.
	\par Parameters:
	<b>ILoad *iload</b>\n\n
	An interface for loading data. */
	IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

	// Put up a dialog that lets the user edit the plug-ins parameters.
	/*! \remarks This method creates and returns a new instance of a class derived from
	<b>EffectParamDlg</b> to manage the user interface. This put up a modal
	dialog that lets the user edit the plug-ins parameters.
	\par Parameters:
	<b>IRendParams *ip</b>\n\n
	This is the interface given to the rendering effect so it may display its
	parameters.
	\return  A new instance of a class derived from <b>EffectParamDlg</b>.\n\n
	Note: <b>typedef SFXParamDlg EffectParamDlg;</b>
	\par Default Implementation:
	<b>{return NULL;}</b> */
	virtual EffectParamDlg *CreateParamDialog(IRendParams *ip) { return NULL; }
	// Implement this if you are using the ParamMap2 AUTO_UI system and the 
	// effect has secondary dialogs that don't have the effect as their 'thing'.
	// Called once for each secondary dialog for you to install the correct thing.
	// Return TRUE if you process the dialog, false otherwise.
	/*! \remarks You should implement this if you are using the ParamMap2
	AUTO_UI system and the effect has secondary dialogs that have something
	other than the incoming effect as their 'thing'. Called once for each
	secondary dialog for you to install the correct thing. Return TRUE if you
	process the dialog, FALSE otherwise, in which case the incoming effect will
	be set into the dialog.\n\n
	Note: Developers needing more information on this method can see the
	remarks for
	<a href="class_mtl_base.html#A_GM_mtlbase_ui">MtlBase::CreateParamDlg()</a>
	which describes a similar example of this method in use (in that case it's
	for use by a texture map plug-in).
	\par Parameters:
	<b>EffectParamDlg* dlg</b>\n\n
	Points to the ParamDlg.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual BOOL SetDlgThing(EffectParamDlg* dlg) { return FALSE; }

	// What G-buffer channels does this Effect require in the output bitmap?
	/*! \remarks Returns a DWORD that indicates the channels that this Effect requires in
	the output bitmap.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which the channels are required.
	\return  The required channels. See \ref gBufImageChannels</a>.
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual DWORD GBufferChannelsRequired(TimeValue t) { return 0; }

	// This is the function that is called to apply the effect.
	/*! \remarks This is the method that is called to apply the effect to the bitmap passed.
	\par Parameters:
	<b>TimeValue t</b>\n\n
	The time at which to apply the effect.\n\n
	<b>Bitmap *bm</b>\n\n
	The bitmap the effect modifies and stores the result in.\n\n
	<b>RenderGlobalContext *gc</b>\n\n
	This can be used to retireve information about the global rendering
	enviornment.\n\n
	<b>CheckAbortCallback *cb</b>\n\n
	Points to an object whose <b>Check()</b> method may be called to determine
	if the user wants to abort. See
	Class CheckAbortCallback. */
	virtual void Apply(TimeValue t, Bitmap* bm, RenderGlobalContext* gc, CheckAbortCallback* cb )=0;

};


//==============================================================================
// Class Effect8
//
//! Extension to class Effect, new to 3ds max 8.
//!
//! This class extends the Effect class through the addition of new methods.
//!
//! All new Effect plugins should derive from the class. All existing Effect
//! should ideally be re-compiled and derived from this class.
//!
//! An Effect8* can easily be retrieved from an Effect& by calling:
//!		Effect8::GetEffect8(Effect& effect)
//!
class Effect8 : public Effect, public BaseInterface {
public:

	//! The interface ID of class Effect8, used in combination with the InterfaceServer
	//! mechanism to retrieve a Effect8* from a Effect*.
	CoreExport static const Interface_ID m_kEffect8_Interface_ID;

	//! Converts an Effect& to an Effect8*, if possible. May return NULL
	//! if the Effect does not derive from class Effect8.
	//! \param[in] effect - The effect for which you want to retrieve the Effect8 interface.
	//! \return A pointer to interface Effect8, if implemented by the effect.
	CoreExport static Effect8* GetEffect8(Effect& effect);

	//! Checks whether the given effect supports the given bitmap. 
	//! By default, all Effects which do not implement Effect8 only support 16bits-per-channel 
	//! bitmaps with alpha (BMM_TRUE_64). The reason for this is that, prior to Max 8, this was the only type 
	//! of bitmap ever used for render output. With Max 8, 32bit floating-point 
	//! bitmaps may be created.
	//! \param[in] effect - The effect which you want to test for compatibilty.
	//! \param[in] bitmap - The bitmap which you want to test for compatibility.
	//! \return true if the given effect supports the given bitmap, false otherwise.
	CoreExport static bool SupportsBitmap(Effect& effect, Bitmap& bitmap);

	//! Returns wether the given bitmap is supported by the effect.
	//! The implementation should usualy check the bitmap type (bitmap.Storage()->Type())
	//! to determine whether it supports that type. 
	//!
	//! Usage example: An effect plugin which uses the BMM_Color_64 version of Bitmap::GetPixels() 
	//! will work with 32bit floating-point bitmaps, but will clamp the colors and should 
	//! therefore be considered incompatible. An effect plugin which uses the BMM_Color_fl version of
	//! Bitmap::GetPixels() can be considered as compatible with all bitmap types.
	//! 
	//! To check whether an effect supports a given bitmap, it is advised to call 
	//! the static method Effect8::SupportsBitmap(Effect&, Bitmap&) 
	//! instead of this. The static method handles class Effect as well as Effect8
	//! and implements appropriate default behaviour for class Effect.
	//!
	//! \param[in] bitmap - The bitmap to be tested for compatibility.
	//! \return true if the bitmap is supported by this render effect, false otherwise.
	virtual bool SupportsBitmap(Bitmap& bitmap) = 0;

	// -- from InterfaceServer
	//! The InterfaceServer mechanism is used to retrieve a Effect8* from a Effect*.
	CoreExport virtual BaseInterface* GetInterface(Interface_ID id);

	// -- from Animatable
	//! The compiler will hide this overload of GetInterface() unless we define it here.
	CoreExport virtual void* GetInterface(ULONG id);
};

// Chunk IDs saved by base class
#define EFFECTBASE_CHUNK	SFXBASE_CHUNK
#define EFFECTNAME_CHUNK	SFXNAME_CHUNK


// --------------------------------
// filter kernel plug-in base class
// --------------------------------

#define AREA_KERNEL_CLASS_ID			0x77912301
#define DEFAULT_KERNEL_CLASS_ID			AREA_KERNEL_CLASS_ID

// Returned by a kernel when it is asked to put up its rollup page.
typedef SFXParamDlg FilterKernelParamDlg;

/*! \sa  Class SpecialFX, Class SFXParamDlg.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This is the plug-in class from which developers sub-class their Anti-aliasing
filters. These filters appear in the Render Scene dialog in the Anti-Aliasing
section in the Filter dropdown list.\n\n
Filter are a very simple plug-in as there are only a few methods that define
them. The <b>KernelFn()</b> method is the one that does the filtering. The
other methods are simply informational.\n\n
A filter kernel is nothing but a curve that starts (usually at 0) at some
distance R from a center pole. This curve is swept around the center pole
forming a volume that is centered at the center of a pixel (0.5,0.5) and
extends to cover some of the near neighboring pixels. The height of this filter
"hat" gives the weight for each pixel under the filter. To achieve high
resolution and good quality this convolution is done at full 8x8 sub-pixel
resolution: each subpixel is weighted by the height of the curve above it. This
is what the KernelFn() method returns. It is given the distance from the center
pole and it returns the weight. This is the only method that computes
anything.\n\n
Note: When things get rasterized in 3ds Max it is done so at a resolution
higher than that of the final output raster. 3ds Max actually rasterizes the
geometry to an 8x8 raster <b>within each</b> pixel. This mask is used do hiding
and shading operations properly <b>inside</b> each pixel. Each of these 64
inside-a-pixel pixels is called a subpixel.\n\n
Theoretically, the <b>KernelFn()</b> function could get called once for each
sub-pixel with the distance of the center of that subpix to the center pole. Of
course that would take a great deal of time. Instead 3ds Max builds a table at
the beginning of a render. This table is slow to build (it requires many calls
to <b>KernelFn()</b>) but fast to use and gives <b>exactly</b> the same answer
as doing the computationly intense approach at the sub-pixel level. Thus the
<b>KernelFn()</b> function can be fairly slow yet the render still happens
relatively fast.
\par Plug-In Information:
Class Defined In RENDER.H\n\n
Super Class ID FILTER_KERNEL_CLASS_ID\n\n
Standard File Name Extension DLK\n\n
Extra Include File Needed None  */
class FilterKernel : public SpecialFX
{
public:
	RefResult NotifyRefChanged(Interval changeInt, 
							   RefTargetHandle hTarget, 
							   PartID& partID, 
							   RefMessage message ) { return REF_SUCCEED; }

	SClass_ID SuperClassID() { return FILTER_KERNEL_CLASS_ID; }
	
	// Saves and loads name. These should be called at the start of
	// a plug-in's save and load methods.
	/*! \remarks Saves the filter name. This should be called at the start of
	a plug-in's save method. */
	IOResult Save(ISave *isave) { return SpecialFX::Save(isave); }
	/*! \remarks Loads the filter name. This should be called at the start of
	a plug-in's load method. */
	IOResult Load(ILoad *iload) { return SpecialFX::Load(iload); }

	// Put up a dialog that lets the user edit the plug-ins parameters.
	virtual FilterKernelParamDlg *CreateParamDialog( IRendParams *ip ) { return NULL; }

	// filter kernel section
	// This is the function that is called to sample kernel values.
	/*! \remarks This is the function that is called to sample the kernel
	values. This returns the weight of the filtering curve at the specified
	distance from the center pole of the curve.
	\par Parameters:
	<b>double x</b>\n\n
	The distance from the center pole of the curve.\n\n
	<b>double y = 0.0</b>\n\n
	The distance from the center pole of the curve in y (for 2D filters). */
	virtual double KernelFn( double x, double y = 0.0 )=0;

	// integer number of pixels from center to filter 0 edge, must not truncate filter
	// x dimension for 2D filters
	/*! \remarks This method returns the kernel 'support'. Support is the
	integer number of pixels <b>beyond</b> the center pixel that are touch in
	some part by the kernel. Support of 0 is 1x1: the area filter. A support
	value of 1 is a 3x3 filter, one pixel on all sides of the center. A support
	of 2 is 5x5. The size of a side of the block is always 2*Support+1. Support
	confides how many pixels might be touched, but not the exact size of the
	filter. */
	virtual long GetKernelSupport()=0;
	// for 2d returns y support, for 1d returns 0
	/*! \remarks For 2D kernels returns the Y support. See
	<b>GetKernelSupport()</b> above. */
	virtual long GetKernelSupportY()=0;

	/*! \remarks Most kernels are 1D and hence circularly symmetric about the
	center pole, but some are 2D like a box or diamond. This method returns
	true if the filter is 2D and false if 1D.\n\n
	A 2D kernel uses both parameters of the methods GetKernelSz() and
	SetKernelSz(). A 1D kernel only uses x; y need not be included in the set.
	Note that GetKernelSz() always requires both x \& y since they are return
	parameters while a 1D kernel ignores y. Also note that a 2D filter provides
	a filter function that uses both the x and y parameters. */
	virtual bool Is2DKernel()=0;
	/*! \remarks Returns true if the filter is variable size; otherwise false.
	Size means the distance from the center pole where the filter function
	becomes essentially 0. In non-variable size filters this width is returned
	in GetKernelSz() and is usually displayed in the greyed out Size box in the
	user interface. In variable size filters get \& set size control the
	bluriness. */
	virtual bool IsVariableSz()=0;
	// 1-D filters ignore the y parameter
	/*! \remarks Stores the kernel size. A 2D kernel stores both parameters of
	this method. A 1D kernel stores only x.
	\par Parameters:
	<b>double x</b>\n\n
	The x value to store.\n\n
	<b>double y = 0.0</b>\n\n
	The y value to store. */
	virtual void SetKernelSz( double x, double y = 0.0 )=0;
	/*! \remarks Retrieves the kernel size. A 2D kernel uses both parameters
	of this method. A 1D kernel only uses x (y is set to 0).
	\par Parameters:
	<b>double\& x</b>\n\n
	The x size is returned here.\n\n
	<b>double\& y</b>\n\n
	The y size is returned here. */
	virtual void GetKernelSz( double& x, double& y )=0;

	// returning true will disable the built-in normalizer
	/*! \remarks Returning true from this method will disable the built-in
	normalizer. Normalized means that if you have some solid color and you
	filter it, you get the same color out;. it is not brighter or darker than
	the original. With positive only filters this is always possible, but with
	some negative lobe filters the colors overflow, so they are toned down
	(produce a slightly darker image, but don't overflow).\n\n
	The normalizer computes the positive and negative volumes of an arbitrary
	filter and scales all the filter values by 1/volume where volume is
	(posVolume - abs( negVolume )). This whole process is turned off and the
	filter values direct from the plug-in have already been scaled internally
	when this method returns true.
	\par Default Implementation:
	<b>{ return FALSE; }</b> */
	virtual bool IsNormalized(){ return FALSE; }

	// this is for possible future optimizations, not sure its needed
	/*! \remarks This method tells the filtering code that it can speed things
	up potentially by dealing with the positive common only case. Currently
	this is not taken advantage of. */
	virtual bool HasNegativeLobes()=0;

	/*! \remarks Returns a pointer to a string which describes the filter.
	This string is displayed in the static text box in the user interface. */
	virtual MCHAR* GetDefaultComment()=0;

	// there are 2 optional 0.0 ...1.0 parameters, for whatever
	/*! \remarks There are two optional parameters that may be used by the
	filter (besides Filter Size). This method returns the number used. If two
	parameters are used both hidden spinners appear in the Anti-aliasing
	section of the Render Scene dialog. If only one parameter is used just the
	top spinner appears. If this method returns nonzero then the methods below
	are used to supply the names for the parameter(s) and to provide and
	receive the values. */
	virtual long GetNFilterParams(){ return 0; }
	/*! \remarks Returns a pointer to the string containing the name of the
	specified parameter.
	\par Parameters:
	<b>long nParam</b>\n\n
	The index of the parameter (0 or 1). */
	virtual MCHAR * GetFilterParamName( long nParam ){ return _M(""); }
	virtual double GetFilterParamMax( long nParam ){ return 1.0; }
	/*! \remarks Returns the specified parameter value.
	\par Parameters:
	<b>long nParam</b>\n\n
	The index of the parameter (0 or 1). */
	virtual double GetFilterParam( long nParam ){ return 0.0; }
	/*! \remarks Stores the value passed for the specified parameter.
	\par Parameters:
	<b>long nParam</b>\n\n
	The index of the parameter (0 or 1).\n\n
	<b>double val</b>\n\n
	The value to set. */
	virtual void SetFilterParam( long nParam, double val ){};
};

#pragma warning(pop)

// Chunk IDs saved by base class
#define FILTERKERNELBASE_CHUNK	0x39bf
#define FILTERKERNELNAME_CHUNK	0x0100


