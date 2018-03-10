
/*==============================================================================
Copyright 2008 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
or which otherwise accompanies this software in either electronic or hard copy form.   
*****************************/
// DESCRIPTION: Interface to the Geometry Checker
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/

#pragma once
#include "stdio.h"
#include "Maxapi.h"
#include "maxheap.h"
#include "ifnpub.h"
#include "iparamb2.h"

//! \brief Base Class For Geometry Checkers

//! The IGeometryChecker class is an abstract base class to be used to create Geometry Checkers
//! GeometryCheckers are basically objects that perform analysis on some INode which then returns
//! the results as a set of geometric indices.
//! \see IGeometryCheckerManager
class IGeometryChecker : virtual public BaseInterface
{
public:

	//! \brief Virtual Deconstructor 
	virtual ~IGeometryChecker(){};

	//! Enums of return types that the geometry checker may return. eFail means the check has failed for some reason,
	//! the other return types specify what type of geometric entity is returned.
	enum ReturnVal{eFail = 0x0, eVertices = 0x1, eEdges = 0x2, eFaces = 0x3};

	//! The returned output of the checker. Currently the output is just a list of indices. What the indices are is
	//! specifed by the ReturnVal enum.
	struct OutputVal
	{
		Tab<int> mIndex;
	};

	//! \brief  Returns whether or not a given node is valid for this geometry checker.
	//! \param[in] node  The node that we will test to see if it is supported. 
	//! \return Return true if the node is valid, otherwise return false if it is not supported.
	virtual bool IsSupported(INode* node) =0;

	//! \brief The type that the geometry checker returns. For now we support vertices, edges and polys or tris.
	//! \return Returns the type of geometric entity that the checker will output. May be IGeometryChecker::eVertices,
	//! IGeometryChecker::eEdges, or IGeometryChecker::eFaces
	virtual ReturnVal TypeReturned() =0;

	//! \brief Perform the geometry check
	//! \param[in] t  The time to run the check
	//! \param[in] node The node to run the check on
	//! \param[out] val The output from the check.
	//! \return The ReturnVal of the check. If the check fails for some reason the returned value will be IGeometryChecker::eFail
	//! otherwise it will be one of the geometric types.
	virtual ReturnVal GeometryCheck(TimeValue t,INode *node, OutputVal &val) =0;

	//! \brief Return the name of the geometry checker. Note that geometry checker name's must be unique.
	virtual MSTR GetName() =0;

	//! \brief Returns the unique identifier of the geometry checker.
	virtual DWORD GetCheckerID() =0;

	//! \brief Returns true if the geometry checker has a property dialog, false otherwise.
	virtual bool HasPropertyDlg() =0;
	//! \brief Show the property dialog.
	virtual void ShowPropertyDlg() =0;


	//! \name Override Functions
	//@{
	//! \brief Returns true if the geometry checker overrides the normal text output, which is usually the number of indices
	//! found by the check, plus the geometric type, e.g. '5 Vertices'
	virtual bool HasTextOverride()=0;
	//! \brief Returns the string that should be displayed in the viewport, rather than the default text output.
	virtual MSTR TextOverride()=0;

	//! \brief Returns true if the geometry checker overrides the normal display otuput, 
	//! which is usually highlighted vertices, edges, or faces.
	virtual bool HasDisplayOverride()=0;
	//!\ brief The display override function used to draw a different display output.
	//!\ param[in] t The time at which to draw the display
	//!\ param[in] node The node that was geometry checked
	//!\ param[in] hwnd The window handle of the viewport where the display should get outputted. Usually the active viewport.
	//!\ param[in] results The last set of results returned by this checker. This may be helpful when drawing the display override.
	virtual void DisplayOverride(TimeValue t, INode* node, HWND hwnd,Tab<int> &results)=0;
	//@}
};


//! \brief Interface ID for the IGeometryCheckerManager Interface
//! \see IGeometryCheckerManager
#define IGEOMETRYCHECKER_MANAGER_INTERFACE Interface_ID(0x3643717a, 0x596f106f)

//! Helper method for getting the IGeometryCheckerManager interface
#define GetIGeometryCheckerManager()	static_cast<IGeometryCheckerManager*>(GetCOREInterface(IGEOMETRYCHECKER_MANAGER_INTERFACE))


//! \brief The interface to the geometry checker manager.

//! This manager is responsible for registering, activitating and displaying the results of the geometry checkers.
//! You get this interface by calling GetIGeometryCheckerManager()
class IGeometryCheckerManager : public FPStaticInterface
{
public:

	//! \name Registeration Functions
	//@{
	//! \brief Register the geometry checker
	//! \param[in] geomChecker The checker to be registered.
	//! \return Returns the index of the checker among the other registered checkers. A value of -1 means an error occured.
	virtual int RegisterGeometryChecker(IGeometryChecker * geomChecker) =0;
	//! \brief Unregister a geometry checker.
	//! \param[in] geomChecker The checker you want to unregister.
	virtual void UnRegisterGeometryChecker(IGeometryChecker *geomChecker) =0;
	//! \brief Returns the number of registered geometry checkers.
	virtual int GetNumRegisteredGeometryCheckers()const =0;
	//@}

	//! \name Properties 
	//@{

	//! \brief Set whether or not the geometry checker system is on so checks will be run and outupt displayed.
	//! \param[in] on  If true the geometry checker system is on, if false it's off and no output or checks occur.
	virtual void SetGeometryCheckerOn(bool on)=0;
	//! \brief Returns whether or not the geometry checker system is on.
	virtual bool GetGeometryCheckerOn()const=0;
	//! \brief Set whether or not the results can be seen through objects or not.
	//! \param[in] val If true the results can be seen through other objects, if false, they can't. In other words
	//! if the value is true we disable z depth testing when drawing the results.
	virtual void SetSeeThrough(bool val) =0;
	//! \brief Get whether or not the results should be seen through objects or not.
	//! \return Return true if the results can be seen through objects, false if not.
	virtual bool GetSeeThrough()const =0;
	//! \brief Turn on or off the automatic updating of the geometry checker. If it's on, then the geometry checker
	//! will run whenever a new object is selected or some change happens to the model. If false the user needs to manual
	//! run and update the checker, which is useful for tests which are slow.
	virtual void SetAutoUpdate(bool val) =0;
	//! \brief Returns true if automatic updating of the geometry checker is on and false if not.
	virtual bool GetAutoUpdate()const =0;
	//! \brief Set the output display text on the top or bottom of the viewport.
	//! \param[in] val If true the text is displayed on top, if false, it's displayed on the bottom.
	virtual void SetDisplayTextUpTop(bool val) =0;
	//! \brief Returns true if the text is displayed on top of the viewport, false if displayed on the bottom.
	virtual bool GetDisplayTextUpTop()const =0;
	//@}

	//! \brief Get the nth GeometryChecker
	//! \param[in] index The index of the registered geoemtry checker
	//! \return Returns the geometry checker at that index
	virtual IGeometryChecker * GetNthGeometryChecker(int index)const =0;
	//! \brief Get the name of the nth Geometry Checker
	//! \param[in] index The index of the registered geoemtry checker
	//! \return The name of the geometry checker at that index
	virtual MSTR GetNthGeometryCheckerName(int index)const =0;

	//! \brief Activate this geometry checker so that it is the one that's run.
	//! \param[in] index  The index of the geometry checker that's being made active.
	//! \return Returns true if the function correctly sets that checker active, false otherwise.
	virtual bool ActivateGeometryChecker( int index) =0;
	//! \brief Returns the index of the activated geometry checker.
	virtual int GetActivatedGeometryCheckerIndex()const =0;

	//! \brief Get whether or not the specified geometry checker has a property dialog.
	//! \param[in] index The index which to check to see if the geometry checker has a dialog.
	//! \return Returns true if that geometry checker has a property dialog, false otherwise.
	virtual bool DoesGeometryCheckerHavePropDlg(int index)const=0;
	//! \brief Show the specified geometry checkers dialog.
	//! \param[in] index The index of the geometry checker that will show it's property dialog.
	virtual void ShowGeometryCheckerPropDlg(int index)=0;

	//! \brief Run the current geometry check based upon the active geometry checker and the current selection set.
	//! \param[in] t The time at which to run the check.
	virtual void RunGeometryCheck(TimeValue t) = 0;

	//! \brief Run the active geometry checker at this time and on this node
	//! \param[in] t The time at which to run the check.
	//! \param[in] node The node on which to run the check.
	//! \param[out] val The output of the check. Currently a list of indices.
	//! \return The ReturnVal of the geometry check. May be IGeometryChecker::eVertices, IGeometryChecker::eEdges,
	//! or IGeometryChecker::eFaces.
	virtual  IGeometryChecker::ReturnVal GeometryCheck(TimeValue t,INode *node, IGeometryChecker::OutputVal &val) = 0;

	//! \brief Show the selection popup menu that lets you pick different checkers to be active and modify different
	//! geometry checker manager properties.
	virtual void PopupMenuSelect() = 0;

	//! \name Getting the Current Output
	//@{
	//! \brief Gets the current ReturnVal of the current geometry check.
	virtual IGeometryChecker::ReturnVal GetCurrentReturnVal()=0;
	//! \brief Gets the number of indices in the current output.
	virtual int GetCurrentOutputCount()=0;
	//! \brief Returns the output string that's shown as text display
	virtual MSTR GetCurrentString() = 0;
	//@}

	//! \brief Select the results of the currently run geometry check on the currently selected node. Based
	//! upon the ReturnType it will select the correct vertices, edges or faces on that node, in the current active modifier or object.
	//! \param[in] t The time to perform the selection.
	virtual void SelectResults(TimeValue t) = 0;


	//! \brief Utility function to use the internal display mechanism to display a set of highlighted vertices, edges or faces.
	//! \param[in] overrideColor The color that you want the items to be displayed in.
	//! \param[in] t The time you want the node to get queried at for display.
	//! \param[in] node The node over which the display will be drawn over.
	//! \param[in] type The type of geometric item that will get displayed, may be IGeometryChecker::eVertices, IGeometryChecker::eEdges,
	//!  or IGeometryChecker::eFaces
	//! \param[in] indices  The indices of the geometric elements that will get highlighted.
	virtual void DisplayResults(Color *overrideColor, TimeValue t,INode *node, HWND hwnd, IGeometryChecker::ReturnVal type,Tab<int> &indices)=0;

	//! \brief Utility function to get the next valid geometry checker id.  New geometry checkers should use this function to get an 
	//! id.
	//! \return returns a valid geometry checker ID.
	virtual DWORD GetNextValidGeometryCheckerID() =0;
};








