/*==============================================================================
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface to Working Pivot Interfaces
// AUTHOR: Michael Zyracki created 2007
//***************************************************************************/

#pragma once
#include "Maxapi.h"
#include "maxheap.h"
#include "ifnpub.h"
#include "iparamb2.h"



//! \brief Interface ID for the IWorkingPivot Interface 
//! \see IWorkingPivot
#define IWORKINGPIVOT_INTERFACE Interface_ID(0x6ea64179, 0x233915fd)

//! Helper method for getting the IWorkingPivot interface
#define GetIWorkingPivot()	static_cast<IWorkingPivot*>(GetCOREInterface(IWORKINGPIVOT_INTERFACE))


//! \brief The interface to the functionality for using the Working Pivot

//! This class contains key functionality for using the working pivot.
//! You get this interface by calling GetIWorkingPivot().
class IWorkingPivot : public FPStaticInterface {

public:

	//! \name Use Mode Functions
	//@{
	/*! This function will put us in and out of the mode where the working pivot becomes the pivot for all objects in the scene.
	\param[in] use If TRUE and we aren't using the working pivot transform space we do so, if FALSE and we are using the working pivot
	transform space then we stop.
	!*/		
	virtual void SetUseMode(BOOL use) = 0;
	
	/*! This function returns whether or not we are using the working pivot transform space, thus whether or not the working pivot is the pivot
	for all of the objects in the scene.
	\return Returns TRUE if we are using the working pivot transform space, FALSE otherwise.
	!*/
	virtual BOOL GetUseMode()const =0;
	//@}

	//! \name Edit Mode Functions
	//@{
	/*! This function will put us in a mode where we can edit and move the working pivots location. While in this mode we can only transform
	the working pivot and not any other objects in the max scene.
	\param[in] edit If TRUE we enter a mode to transform the working pivot, if FALSE we leave the working pivot edit mode.
	!*/
	virtual void SetEditMode(BOOL edit) = 0;
	
	/*!This function gets whether or not we are in the mode where we can edit the working pivot.
	//!\ return Return TRUE if we are in the mode where we can edit the working pivot, otherwise return FALSE.
	!*/
	virtual BOOL GetEditMode()const =0;

	/*! This function puts us in a command mode where any click in the viewport will automatically move the working pivot to that position in screen space.
	\param place If TRUE and not in the place pivot view command mode then we enter the place pivot view command mode, if FALSE and we are in the place pivot
	view command mode then we exit that mode.  Note that we aren't in the edit working pivot mode when entering this mode we enter edit working pivot mode automatically
	and then if we exit in this case we also turn off edit working pivot mode.
	\param alignToView If TRUE and we are entering this mode then when we place the pivot it also automatically aligns to the current view of the viewport when you click. 
	Also note that in this case it also sets the Align To View toggle in the 'Place Pivot To:' group in the Hierarchy panel.
	!*/
	virtual void SetPlacePivotViewMode(BOOL place,BOOL alignToView) = 0;

	/*! This function returns whether or not we are in the place pivot command mode
	\return Returns TRUE if we are in the place pivot command mode, otherwise returns FALSE.  Note that we will never be in the place pivot mode if not also in the edit working
	pivot mode.
	!*/
	virtual BOOL GetPlacePivotViewMode()const = 0;

	/*! This function puts us in a command mode where we will place the working pivot onto any surface in the scene, including grids.
	\param place If TRUE and not in the place pivot on a surface command mode then we enter the place pivot on a surface command mode, if FALSE and we are
	in this command mode then we exit this mode.  Note that we aren't in the edit working pivot mode when entering this mode then we enter
	edit working pivot mode automatically and then if we exit in this case we also turn off edit working pivot mode.
	\param alignToView If TRUE and we are entering this mode then when we place the pivot by clicking it also automatically aligns to the current view of the viewport when you click.
	Also note that in this case it also sets the Align To View toggle in the 'Place Pivot To:' group in the Hierarchy panel.
	!*/
	virtual void SetPlacePivotSurfaceMode(BOOL place,BOOL alignToView) = 0;

	/*! This function returns whether or not we are in the place pivot on the auto grid command mode
	\return Returns TRUE if we are in the place pivot on the auto grid command mode, otherwise returns FALSE.  Note that we will never be in the 
	place pivot on the auto grid mode if not also in the edit working pivot mode.
	!*/
	virtual BOOL GetPlacePivotSurfaceMode()const = 0;


	/*! This function returns whether or not when we are in a place pivot mode if we are also aligning to the view.
	\return Returns TRUE if we are in a place pivot mode and we are aligning to the view, otherwise if we aren't in a place pivot mode
	or we are but we aren't aligning to the view it returns FALSE. 
	!*/	
	virtual BOOL GetPlacePivotAlignToView() const = 0;

	//@}
	
	//! \name Transform Functions
	//@{
	/*! This functions sets the world space transform of the working pivot. When using the working pivot this will be the location about which pivoting
	will occur, and when editing the working pivot this is the location of where the pivot is when the mode is entered and exited.
	\param [in] mat The world space location of the working pivot.
	!*/
	virtual void SetTM(Matrix3 &mat) =0;

	/*!  This function gets the world space transofrm of the working pivot.  When using the working pivot this will be the location about which pivoting
	will occur, and when editing the working pivot this is the location of where the pivot is when the mode is entered and exited. 
	\return Returns the world space location of the working pivot.
	!*/
	virtual Matrix3 GetTM()const = 0;
	//@}

	//! \name Display Functions
	//@{
	/*! This function sets the axis display size of the working pivot when it's displayed when in use or being edited.
	\param size The length of each working pivot axis in 3ds Max units.
	!*/
	virtual void SetAxisSize(float size) = 0;
	
	/*! This function gets the axis display size of the working pivot when it's displayed when in use or being edited.
	\return Returns the length of each working pivot axis in 3ds Max units.
	!*/
	virtual float GetAxisSize()const =0;
	
	//@}

	//! \name Transformation Convenience Functions
	//@{
	/*! This function aligns the working pivot transform to the that of the active viewport. It does this by changing the rotation of the
	working pivot transform to match the rotation of the active viewport.  Note that this function does nothing if we aren't in the edit or
	use working pivot modes.
	!*/
	virtual void AlignToView() =0;
	/*! This function allows you to reset the working pivot's transform so that it matches that of the current selection. After this function is called,
	the working pivots world space transformation will equal that of the current selection. Also note that this function does nothing if we
	aren't in the edit or use working pivot	modes.  
	!*/
	virtual void ResetTM() =0;
	/*! This function allows you to reset the working pivot's transform so that it matches that of a selected node that's passed in as a parameter.
	\param node The node whose world space transform the working pivot's transofrm will get set equal to.  If the node is NULL, or if
	we aren't in the edit or use working pivot modes, then nothing will happen.  Finally, the working pivot will match up with the node transform,
	not the object trnasform.
	!*/
	virtual void ResetTM(INode *node)=0;
	//@}


	//!\name FileIO (Internal Use Only)
	//@{
	//! \brief Used internally
	virtual IOResult Save(ISave* iSave) = 0;
	//! \brief Used internally
	virtual IOResult Load(ILoad* iLoad) = 0;
	//@}


};




