/**********************************************************************
 *<
	FILE: IParamM.h

	DESCRIPTION:  Parameter Maps

	CREATED BY: Rolf Berteig

	HISTORY: created 10/10/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "maxtypes.h"
#include "coreexp.h"
#include "custcont.h" // for EditSpinnerType
#include "paramtype.h" // for ControlType

// forward declarations
class IParamMap;
class IRendParams;
class IParamArray;
class ParamDimension;
CoreExport extern ParamDimension* defaultDim;

// If custom handling of controls needs to be done, ParameterMap
// client can't implement one of these and set is as the ParameterMap's
// user callback.
/*! \sa  Class IParamMap.
\par Description:
This class is used with parameter maps. If there is some custom handling
required by a particular control, the client can derive a class from
<b>ParamMapUserDlgProc</b> and set it as the parameter map's user callback
(usually using <b>SetUserDialogProc()</b>).  */
class ParamMapUserDlgProc: public MaxHeapOperators {
	public:
		/*! Destructor. */
		virtual ~ParamMapUserDlgProc() {}
		/*! \remarks This is the dialog proc that will be called to process the control messages.
		This proc will be called after the default processing is complete.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		This is the current time.\n\n
		<b>IParamMap *map</b>\n\n
		This is a pointer to the parameter map.\n\n
		<b>HWND hWnd</b>\n\n
		This is the handle to the dialog.\n\n
		<b>UINT msg</b>\n\n
		This is the message that should be processed by the dialog proc.\n\n
		<b>WPARAM wParam</b>\n\n
		This is a parameter that holds message specific information.\n\n
		<b>LPARAM lParam</b>\n\n
		This is a parameter that holds message specific information.
		\return  This is essentially the equivalent of a normal Windows dialog proc, so
		it should return whatever value a normal dialog proc returns for the message.
		An exception is that the value <b>REDRAW_VIEWS</b> may be returned to cause the
		viewports to be redrawn. */
		virtual INT_PTR DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)=0;
		/*! \remarks If the DlgProc is non-NULL when the ParamMap is deleted the
		DeleteThis() method will be called. This method is usually implemented
		as follows: <b>void DeleteThis() {delete this;}</b> */
		virtual void DeleteThis()=0;
		/*! \remarks This method is available in release 2.0 and later
		only.\n\n
		This method is called whenever the dialog is being updated. If the
		parameter map is invalidated, 3ds Max will update the user interface.
		When it does, this method is called so a developer may do anything they
		need to on each update.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which the update is taking place.
		\par Default Implementation:
		<b>{}</b>\n\n
		\sa  For more information on Dialog Procs see the Advanced Topics
		section on
		<a href="ms-its:3dsmaxsdk.chm::/ui_custom_controls.html">Custom Controls</a>. */
		virtual void Update(TimeValue t) { UNUSED_PARAM(t); }
	};

// Return this from DlgProc to get the viewports redrawn.
#define REDRAW_VIEWS	2



/*! \sa ,
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>, Class ParamMapUserDlgProc, Class ParamUIDesc, <a href="ms-its:3dsmaxsdk.chm::/paramblocks2_root.html">Parameter Blocks</a>, Class IParamArray\n\n
\par Description:
This class provides methods to work with parameter maps. These are things like
invalidating the parameter map so it gets redrawn, working with the parameter
blocks associated with the parameter map, and establishing an optional dialog
proc to handle controls not directly handled by the pmap. This section also
documents several functions that are available for creating and destroying
parameter maps but are not part of this class.\n\n
Note: The use of this class requires the explicit inclusion the
<b>IPARAMM.H</b> header file. */
class IParamMap: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~IParamMap() {}

	 	// Mark the UI as needing to be updated.
	 	/*! \remarks Implemented by the System.\n\n
	 	Call this method to update (redraw) the user interface controls. This
	 	marks the UI controls as needing to be updated and the parameter map
	 	will take care of it. */
	 	virtual void Invalidate()=0;
		
		// Swaps the existing parameter block with a new one and updates UI.
		/*! \remarks Implemented by the System.\n\n
		This method swaps the existing parameter block with a new one and
		updates the user interface. Consider the following example to
		understand how this is used: If a user is in create mode, and has
		created a sphere object, and then goes to create another sphere, the
		user interface stays up. The parameter map that manages the UI is not
		deleted. When the user creates the second sphere, the parameter map
		needs to refer to the new sphere's parameter block (not the previous
		one any longer). This method is used to set the parameter map to point
		to the new parameter block.
		\par Parameters:
		<b>IParamArray *pb</b>\n\n
		A pointer to the new parameter block. */
		virtual void SetParamBlock(IParamArray* pb)=0;

		// The given proc will be called _after_ default processing is done.
		// The callback can then apply constraints to controls.
		// Note that if the proc is non-NULL when the ParamMap is deleted
		// its DeleteThis() method will be called.
		/*! \remarks Implemented by the System.\n\n
		This method allows the developer to provide special handling for a
		control. The developer provides a dialog proc to process the message
		from the control. This method is used to tell the parameter map that
		the developer defined method should be called. The given proc will be
		called <b>after</b> default processing is done. Note that if the proc
		is non-NULL when the ParamMap is deleted its <b>DeleteThis()</b> method
		will be called.
		\par Parameters:
		<b>ParamMapUserDlgProc *proc=NULL</b>\n\n
		A pointer to the user dialog proc class to process the control. */
		virtual void SetUserDlgProc(ParamMapUserDlgProc *proc=NULL)=0;
		virtual ParamMapUserDlgProc *GetUserDlgProc()=0;

		// Changes a map entry to refer to a different item in the parameter block.
		/*! \remarks Implemented by the System.\n\n
		This method changes a parameter map entry to refer to a different item
		in the parameter block. This is used for example by the Optimize
		modifier. This modifier has two sets of parameters that may be adjusted
		(L1 and L2). Optimize only maintains a single parameter block however.
		This pblock contains both sets of parameters. When the user switches
		between these two sets, this method is called to point the UI controls
		at different indices in the parameter block.
		\par Parameters:
		<b>int mapIndex</b>\n\n
		The map entry to change.\n\n
		<b>int blockIndex</b>\n\n
		The new parameter block index. */
		virtual void SetPBlockIndex(int mapIndex, int blockIndex)=0;

		// Access the dialog window.
		/*! \remarks Implemented by the System.\n\n
		Returns the window handle of the rollup page (or dialog). */
		virtual HWND GetHWnd()=0;

		// Access the parameter block
		/*! \remarks Implemented by the System.\n\n
		Returns a pointer to the parameter block managed by the parameter map.
		*/
		virtual IParamArray *GetParamBlock()=0;

		// Is the dialog proc active
		virtual BOOL DlgActive()=0;
	};




// Giving this value for scale specifies autoscale
#define SPIN_AUTOSCALE	-1.0f

/*! \sa  Class IParamMap, <a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>.
\par Description:
The ParamUIDesc class is used in conjunction with the parameter maps mechanism.
It is used for creating descriptors that define the properties of a user
interface control such as its type (spinner, radio button, check box, etc.),
which resource ID it refers to, and which index into the virtual array of
parameters it uses. See the Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/parammap1_root.html">Parameter Maps</a>
for an overview of how these descriptors are used.   */
class ParamUIDesc: public MaxHeapOperators {
	public:
		// Float or int controlled by a single spinner
		/*! <b>float lowLim,float highLim,float scale,</b>\n\n
		<b>ParamDimension *dim=defaultDim);</b> \remarks Constructor. This constructor
		is used for a float or int controlled by a single spinner control:
		\par Parameters:
		<b>int index</b>\n\n
		This is the index into the IParamArray virtual array of this UI control.\n\n
		<b>EditSpinnerType spinType</b>\n\n
		This parameter specifies the type of value which may be entered. The valid
		types (listed in <b>CUSTCONT.H</b>) are:\n\n
		* <b>EDITTYPE_INT</b> - Any integer value.\n\n
		* <b>EDITTYPE_FLOAT</b> - Any floating point value.\n\n
		* <b>EDITTYPE_UNIVERSE</b> - This is a value in world space units. It respects
		the systems unit settings (for example feet and inches).\n\n
		* <b>EDITTYPE_POS_INT</b> - Any integer \>= 0\n\n
		* <b>EDITTYPE_POS_FLOAT</b> - Any floating point value \>= 0.0\n\n
		* <b>EDITTYPE_POS_UNIVERSE</b> - This is a positive value in world space units.
		It respects the systems unit settings (for example feet and inches) .\n\n
		* <b>EDITTYPE_TIME</b> - This is a time value. It respects the system time
		settings (SMPTE for example).\n\n
		<b>int idEdit, int idSpin</b>\n\n
		These are the resource IDs the edit control and the spinner control.\n\n
		<b>float lowLim</b>\n\n
		This is the minimum value the spinner can take on.\n\n
		<b>float highLim</b>\n\n
		This is the maximum value the spinner can take on.\n\n
		<b>float scale</b>\n\n
		This is the increment or decrement value used when the user uses the up or down
		arrow buttons of the spinner control. You may also pass the value
		<b>SPIN_AUTOSCALE</b>. This causes 3ds Max to automatically adjust the value
		used to increment or decrement based on the current value being edited. This
		allows the spinner to cover a wider range of values with less mouse movement or
		button clicking from the user. */
		CoreExport ParamUIDesc(
			int index,EditSpinnerType spinType,int idEdit,int idSpin,
			float lowLim,float highLim,float scale,ParamDimension* dim = defaultDim);

		// int controlelled by n radio buttons
		// vals[i] represents the value if ctrlIDs[i] is checked.
		// if vals=NULL then ctrlIDs[i] represents a value of i.
		//
		// OR
		// 
		// int controlled by multiple check boxes where each
		// check boxes controlls a single bit.
		// vals[i] specifies which bit ctrlIds[i] controls.
		// If vals=NULL ctrlIDs[i] controls the ith bit.		
		/*! \remarks Constructor. This constructor is used for an <b>int</b>
		controlled by n radio buttons where\n\n
		<b>vals[i]</b> represents the value if <b>ctrlIDs[i]</b> is checked. If
		<b>vals</b>=NULL then <b>ctrlIDs[i]</b> represents a value of i.\n\n
		Or it may be used for:\n\n
		An <b>int</b> controlled by multiple check boxes where each check boxes
		controls a single bit.\n\n
		<b>vals[i]</b> specifies which bit <b>ctrlIds[i]</b> controls. If
		<b>vals</b>=NULL then <b>ctrlIDs[i]</b> controls the i-th bit.
		\par Parameters:
		<b>int index</b>\n\n
		This is the index into the IParamArray virtual array of this UI
		control.\n\n
		<b>ControlType type</b>\n\n
		This specifies the type of control to use. The available control
		are:\n\n
		* <b>TYPE_RADIO</b> - Radio Buttons.\n\n
		* <b>TYPE_MULTICHEKBOX</b> - Multiple Check Boxes. Note: This option is
		not currently supported.\n\n
		<b>int *ctrlIDs</b>\n\n
		An array of control IDs. See the Remarks above.\n\n
		<b>int count</b>\n\n
		This is the number of control IDs in the array above.\n\n
		<b>int *vals=NULL</b>\n\n
		An array of values. See the Remarks above. */
		CoreExport ParamUIDesc(
			int index, ControlType type, int* ctrlIDs, int count, int* vals = NULL);

		// int controlled by a single check box (BOOL)
		// or Point3 controlled by a color swatch.
		/*! \remarks Constructor. This version is used for an int controlled
		by a single check box (BOOL) or a Point3 controlled by a color swatch.
		\par Parameters:
		<b>int index</b>\n\n
		This is the index into the IParamArray virtual array of this UI
		control.\n\n
		<b>ControlType type</b>\n\n
		This specifies the type of control to use. The available control types
		(defined in <b>IPARAM.H</b>) are:\n\n
		* <b>TYPE_SPINNER</b> - Spinner Control.\n\n
		* <b>TYPE_RADIO</b> - Radio Button.\n\n
		* <b>TYPE_SINGLECHEKBOX</b> - Single Check Box.\n\n
		* <b>TYPE_MULTICHEKBOX</b> - Multiple Check Boxes. Note: This option is
		not currently supported.\n\n
		* <b>TYPE_COLORSWATCH</b> - Color Swatch.\n\n
		<b>int id</b>\n\n
		This is the resource ID of the control. */
		CoreExport ParamUIDesc(int index,ControlType type,int id);

		// Point3 controlled by 3 spinners
		/*! <b>int idSpin3,float lowLim,float highLim,float scale,</b>\n\n
		<b>ParamDimension *dim=defaultDim);</b> \remarks Implemented by the
		System.\n\n
		This version if for a Point3 controlled by 3 spinners
		\par Parameters:
		<b>int index</b>\n\n
		This is the index into the IParamArray virtual array of this UI control.\n\n
		<b>EditSpinnerType spinType</b>\n\n
		This parameter specifies the type of value which may be entered. The valid
		types (listed in <b>CUSTCONT.H</b>) are:\n\n
		* <b>EDITTYPE_INT</b> - Any integer value.\n\n
		* <b>EDITTYPE_FLOAT</b> - Any floating point value.\n\n
		* <b>EDITTYPE_UNIVERSE</b> - This is a value in world space units. It respects
		the systems unit settings (for example feet and inches).\n\n
		* <b>EDITTYPE_POS_INT</b> - Any integer \>= 0\n\n
		* <b>EDITTYPE_POS_FLOAT</b> - Any floating point value \>= 0.0\n\n
		* <b>EDITTYPE_POS_UNIVERSE</b> - This is a positive value in world space units.
		It respects the systems unit settings (for example feet and inches) .\n\n
		* <b>EDITTYPE_TIME</b> - This is a time value. It respects the system time
		settings (SMPTE for example).\n\n
		<b>int idEdit1,int idSpin1</b>\n\n
		These are the resource IDs of the first edit and spinner controls.\n\n
		<b>int idEdit2,int idSpin2</b>\n\n
		These are the resource IDs of the second edit and spinner controls.\n\n
		<b>int idEdit3,int idSpin3</b>\n\n
		These are the resource IDs of the third edit and spinner controls.\n\n
		<b>float lowLim</b>\n\n
		This is the minimum value the spinner can take on.\n\n
		<b>float highLim</b>\n\n
		This is the maximum value the spinner can take on.\n\n
		<b>float scale</b>\n\n
		This is the increment or decrement value used when the user uses the up or down
		arrow buttons of the spinner control. You may also pass the value
		<b>SPIN_AUTOSCALE</b>. This causes 3ds Max to automatically adjust the value
		used to increment or decrement based on the current value being edited. This
		allows the spinner to cover a wider range of values with less mouse movement or
		button clicking from the user.\n\n
		<b>ParamDimension *dim=defaultDim</b>\n\n
		This parameter represents the type and magnitude of the parameter. See
		Class ParamDimension. */
		CoreExport ParamUIDesc(int index,
				EditSpinnerType spinType,
				int idEdit1,int idSpin1,
				int idEdit2,int idSpin2,
				int idEdit3,int idSpin3,
				float lowLim, float highLim, float scale,
				ParamDimension* dim = defaultDim);
		
		
		int	pbIndex;
		ParamType 	ptype;
		ControlType	ctype;
		int id[6];
		int *ids;
		int *vals;
		int count;

		EditSpinnerType spinType;
		float lowLim;
		float highLim;
		float scale;
		ParamDimension *dim;
	};

// Creates a parameter map that will handle a parameter block in a modeless
// dialog where time does not change and the viewport is not redrawn.
// Note that there is no need to destroy it. It executes the dialog and then
// destorys itself. Returns TRUE if the user selected OK, FALSE otherwise.
/*! \remarks This function creates a parameter map that will handle a
parameter block in a modal dialog where time does not change and the viewport
is not redrawn. Note that there is no need to destroy it. It executes the
dialog and then destroys itself.
\par Parameters:
<b>ParamUIDesc *desc</b>\n\n
The array of ParamUIDescs, one element for each control to be managed.\n\n
<b>int count</b>\n\n
The number of items in the array above.\n\n
<b>IParamArray *pb</b>\n\n
Pointer to an instance of the class IParamArray. This is the pointer to the
virtual array of parameters.\n\n
<b>TimeValue t</b>\n\n
This is just the current time when the user is bringing up the dialog.\n\n
<b>HINSTANCE hInst</b>\n\n
The DLL instance handle of the plug-in\n\n
<b>MCHAR *dlgTemplate</b>\n\n
Dialog template for the dialog box.\n\n
<b>HWND hParent</b>\n\n
The parent window handle.\n\n
<b>ParamMapUserDlgProc *proc=NULL</b>\n\n
If there is some custom handling required by a particular control, the client
can derive a class from <b>ParamMapUserDlgProc</b> and set it as the parameter
map's user callback. See Class ParamMapUserDlgProc.
\return  TRUE if the user selected OK; otherwise FALSE. */
CoreExport BOOL CreateModalParamMap(
		ParamUIDesc *desc,int count,
		IParamArray *pb,
		TimeValue t,
		HINSTANCE hInst,
		MCHAR *dlgTemplate,
		HWND hParent,
		ParamMapUserDlgProc *proc=NULL);


// Creates a parameter map to handle the display of parameters in the command panal.
// 
// This will add a rollup page to the command panel.
// DestroyCPParamMap().
//
/*! \remarks Creates a parameter map to handle the display of parameters in
the command panel. This will add the rollup page to the command panel.
\par Parameters:
<b>ParamUIDesc *desc</b>\n\n
The array of ParamUIDescs, one element for each control to be managed.\n\n
<b>int count</b>\n\n
The number of items in the array above.\n\n
<b>IParamArray *pb</b>\n\n
Pointer to an instance of the class IParamArray. This is the pointer to the
virtual array of parameters.\n\n
<b>Interface *ip</b>\n\n
The interface pointer passed into the BeginEditParams() method.\n\n
<b>HINSTANCE hInst</b>\n\n
The DLL instance handle of the plug-in\n\n
<b>MCHAR *dlgTemplate</b>\n\n
Dialog template for the rollup page (created using the resource editor)\n\n
<b>MCHAR *title</b>\n\n
The title displayed in the rollup page title bar.\n\n
<b>DWORD flags</b>\n\n
A set of flags to control settings of the rollup page.\n\n
<b>APPENDROLL_CLOSED</b>\n\n
Starts the page in the rolled up state.
\return  A pointer to the parameter map that is created. */
CoreExport IParamMap *CreateCPParamMap(
		ParamUIDesc *desc,int count,
		IParamArray *pb,
		Interface *ip,
		HINSTANCE hInst,
		MCHAR *dlgTemplate,
		MCHAR *title,
		DWORD flags);


/*! \remarks This method allows one to switch rollups in the command panel. 
It creates the new parameter map and calls Interface:: ReplaceRollupPage().
\par Parameters:
<b>HWND oldhw</b>\n\n
The window handle of the old rollup.\n\n
<b>ParamUIDesc *desc</b>\n\n
The array of ParamUIDescs, one element for each control to be managed.\n\n
<b>int count</b>\n\n
The number of items in the array above.\n\n
<b>IParamArray *pb</b>\n\n
Pointer to an instance of the class IParamArray. This is the pointer to the
virtual array of parameters.\n\n
<b>Interface *ip</b>\n\n
The interface pointer passed into the BeginEditParams() method.\n\n
<b>HINSTANCE hInst</b>\n\n
The DLL instance handle of the plug-in\n\n
<b>MCHAR *dlgTemplate</b>\n\n
Dialog template for the rollup page (created using the resource editor)\n\n
<b>MCHAR *title</b>\n\n
The title displayed in the rollup page title bar.\n\n
<b>DWORD flags</b>\n\n
A set of flags to control settings of the rollup page.\n\n
<b>APPENDROLL_CLOSED</b>\n\n
Starts the page in the rolled up state.
\return  A pointer to the parameter map that is created. */
CoreExport IParamMap *ReplaceCPParamMap(
		HWND oldhw,
		ParamUIDesc *desc,int count,
		IParamArray *pb,
		Interface *ip,
		HINSTANCE hInst,
		MCHAR *dlgTemplate,
		MCHAR *title,
		DWORD flags);

/*! \remarks This function destroys a command panel parameter map. The rollup
page from the command panel is removed and the parameter map is deleted.
\par Parameters:
<b>IParamMap *m</b>\n\n
A pointer to the parameter map to destroy. */
CoreExport void DestroyCPParamMap(IParamMap *m);


// Creates a parameter map to handle the display of render parameters or
// atmospheric plug-in parameters.
/*! \remarks This function creates a parameter map to handle the display of
render parameters or atmospheric plug-in parameters.
\par Parameters:
<b>ParamUIDesc *desc</b>\n\n
The array of ParamUIDescs, one element for each control to be managed.\n\n
<b>int count</b>\n\n
The number of items in the array above.\n\n
<b>IParamArray *pb</b>\n\n
Pointer to an instance of the class IParamArray. This is the pointer to the
array of parameters.\n\n
<b>IRendParams *ip</b>\n\n
The interface pointer passed into <b>CreateParamDlg()</b>.\n\n
<b>HINSTANCE hInst</b>\n\n
The DLL instance handle of the plug-in\n\n
<b>MCHAR *dlgTemplate</b>\n\n
Dialog template for the rollup page (created using the resource editor)\n\n
<b>MCHAR *title</b>\n\n
The title displayed in the rollup page title bar.\n\n
<b>DWORD flags</b>\n\n
A flag to control the settings of the rollup page:\n\n
<b>APPENDROLL_CLOSED</b>\n\n
Starts the page in the rolled up state.
\return  A pointer to the parameter map that is created. */
CoreExport IParamMap *CreateRParamMap(
		ParamUIDesc *desc,int count,
		IParamArray *pb,
		IRendParams *ip,
		HINSTANCE hInst,
		MCHAR *dlgTemplate,
		MCHAR *title,
		DWORD flags);
/*! \remarks This function destroys a parameter map created by
<b>CreateRParamMap()</b>. The rollup page is removed and the parameter map is
deleted.
\par Parameters:
<b>IParamMap *m</b>\n\n
A pointer to the parameter map to destroy. */
CoreExport void DestroyRParamMap(IParamMap *m);

class IMtlParams;

// Creates a parameter map to handle the display of texture map or
// material parameters in the material editor.
/*! \remarks This function is available in release 2.0 and later only.\n\n
This function creates a parameter map to handle the display of texture map or
material parameters in the material editor.
\par Parameters:
<b>ParamUIDesc *desc</b>\n\n
The array of ParamUIDescs, one element for each control to be managed.\n\n
<b>int count</b>\n\n
The number of items in the array above.\n\n
<b>IParamArray *pb</b>\n\n
Pointer to an instance of the class IParamArray. This is the pointer to the
array of parameters.\n\n
<b>IMtlParams *ip</b>\n\n
The interface pointer. See Class IMtlParams.\n\n
<b>HINSTANCE hInst</b>\n\n
The DLL instance handle of the plug-in\n\n
<b>MCHAR *dlgTemplate</b>\n\n
Dialog template for the rollup page (created using the resource editor)\n\n
<b>MCHAR *title</b>\n\n
The title displayed in the rollup page title bar.\n\n
<b>DWORD flags</b>\n\n
A flag to control the settings of the rollup page:\n\n
<b>APPENDROLL_CLOSED</b>\n\n
Starts the page in the rolled up state.
\return  A pointer to the parameter map that is created. */
CoreExport IParamMap *CreateMParamMap(
		ParamUIDesc *desc,int count,
		IParamArray *pb,
		IMtlParams *ip,
		HINSTANCE hInst,
		MCHAR *dlgTemplate,
		MCHAR *title,
		DWORD flags);
/*! \remarks This function is available in release 2.0 and later only.\n\n
This function destroys a parameter map created by <b>CreateMParamMap()</b>. The
rollup page is removed and the parameter map is deleted.
\par Parameters:
<b>IParamMap *m</b>\n\n
A pointer to the parameter map to destroy.  */
CoreExport void DestroyMParamMap(IParamMap *m);






