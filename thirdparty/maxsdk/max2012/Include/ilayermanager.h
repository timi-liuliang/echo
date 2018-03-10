/**********************************************************************
 *<
	FILE: ILayerManager.h

	DESCRIPTION: Declaration of the ILayerManager interface

	CREATED BY:	Peter Sauerbrei

	HISTORY: Created 19 October 1998

 *>	Copyright (c) 1998-99, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "ref.h"

class ILayer;
class LayerIterator;
class ConstLayerIterator;

//! \brief This class is an interface to the layer manager.
/*! Note that some methods of this class are not functional in 3ds Max (only in 3D Studio VIZ).
To get a pointer to the global instance of this class use the following code:
\code
	ILayerManager* manager = GetCOREInterface13()->GetLayerManager();
\endcode
\sa  Class ReferenceTarget, Class ILayer, Class Interface13
*/
class ILayerManager : public ReferenceTarget
{
public:
	//! The super class ID of the layer manager interface
	static const SClass_ID kLayerManagerSuperClassID;

	// From Animatable
	SClass_ID SuperClassID() { return kLayerManagerSuperClassID; }

	// local methods ---------------------------
	/*! \remarks Adds the specified layer.
	\par Parameters:
	<b>ILayer *layer</b>\n\n
	Points to the layer to add.
	\return  Returns true if the layer was added; false if not. */
	virtual bool AddLayer(ILayer * layer) = 0;
	/*! \remarks Creates a layer. The name is based on the incremented layer
	count. */
	virtual ILayer * CreateLayer(void) = 0;					// creates a new layer
	virtual ILayer * CreateLayer(MSTR& name) = 0;		    // creates a new layer given a name
	/*! \remarks Deletes the layer whose name is passed. 
		\param MSTR name - The name for the layer.
		\return  TRUE if the layer was deleted, otherwise FALSE. */
	virtual BOOL DeleteLayer(const MSTR & name) = 0;
	/*! \remarks Sets the layer whose name is passed as current.
	\par Parameters:
	<b>const MSTR \&name</b>\n\n
	The name for the new current layer. */
	virtual void SetCurrentLayer(const MSTR & name) = 0;	// sets the current layer
	/*! \remarks Sets the current layer based on the selection set (the common
	layer). */
	virtual void SetCurrentLayer(void) = 0;
	/*! \remarks Returns an interface to the current layer. */
	virtual ILayer * GetCurrentLayer(void) const = 0;		// gets the current layer
	/*! \remarks Edits the layer whose name is passed. Note: This method does
	nothing in 3ds Max.
	\par Parameters:
	<b>const MSTR \&name</b>\n\n
	The name of the layer to edit. */
	virtual void EditLayer(const MSTR & name) = 0;
	/*! \remarks Brings up the layer property dialog. Note: This method does
	nothing in 3ds Max.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle. */
	virtual void DoLayerPropDialog(HWND hWnd) = 0;
	/*! \remarks Returns the number of layers. */
	virtual int GetLayerCount(void) = 0;
	//! Gets the specified layer by index
	virtual ILayer* GetLayer(int) const = 0;
	/*! \remarks Returns a pointer to a layer interface for the named layer.
	\par Parameters:
	<b>const MSTR \&name</b>\n\n
	The name of the layer to get. */
	virtual ILayer* GetLayer(const MSTR & name) const = 0;
	/*! \remarks Brings up the select layer dialog. Note: This method does
	nothing in 3ds Max.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle. */
	virtual void DoLayerSelDialog(HWND hWnd) = 0;
	//virtual void SetupToolList(HWND hWnd) = 0;
	/*! \remarks Sets up the toolbar list.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The parent window handle. */
	virtual void SetupToolList2(HWND hWnd, HWND hParent) = 0;
	/*! \remarks Extends the right click menu. Note: This method does nothing
	in 3ds Max.
	\par Parameters:
	<b>HMENU hMenu</b>\n\n
	The handle of the menu to append to.\n\n
	<b>bool geometry = true</b>\n\n
	Use true to add the geometry commands; false to not add them.\n\n
	<b>bool grid = false</b>\n\n
	Use true to add the grid commands; false to not add them. */
	virtual void ExtendMenu(HMENU hMenu, bool geometry = true, bool grid = false) = 0;
	virtual MSTR GetSavedLayer(int i) const = 0;
	/*! \remarks Returns an interface to the 0 layer. */
	virtual ILayer * GetRootLayer() const = 0;
	/*! \remarks Resets the layer manager.
	\par Parameters:
	<b>BOOL fileReset = FALSE</b>\n\n
	This parameter is ignored. */
	virtual void Reset(BOOL fileReset = FALSE) = 0;
	/*! \remarks This method will bring up the select objects by layer dialog.
	\par Parameters:
	<b>HWND hWnd</b>\n\n
	The handle to the parent window. */
	virtual void SelectObjectsByLayer(HWND hWnd) = 0;

	// new Hide/Freeze logic, 030516  --prs.
	virtual void SetPropagateToLayer(int prop) = 0;
	virtual int GetPropagateToLayer(void) = 0;
};

