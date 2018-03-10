/**********************************************************************
 *<
	FILE: EditSoftSelectionMode.h

	DESCRIPTION:  Edit Soft Selection Command Mode

	CREATED BY: Michael Zyracki

	HISTORY: created September 2007

 *>	Copyright (c) 2007-2008 Autodesk, Inc., All Rights Reserved.
 **********************************************************************/
#pragma once

#include "object.h"
#include "Maxapi.h"
#include "maxheap.h"
#include "cmdmode.h"
#include "objmode.h"

//! Edit Soft Selection Callback used to control the new Edit Soft Selection Command Mode
/*! New to 3ds max 2009, the new pure virtual base class, EditSSCB, is to be used to interact with the new
Edit Soft Selection CommandMode which let's you interactively set soft selection's falloff, pinch and bubble parameters.
The normal use will be to create a subclass of this class which implements the given pure virtual functions
and then pass this object it in as a parameter to the EditSSMode constructor. \see EditSSMode.  These overriden functions
will then be called as the command mode operates.   An example of using this command mode may be found in the 
EditPolyMod project found in .\maxsdk\samples\mesh\EditPolyMod.
*/
class EditSSCB :  public MaxHeapOperators
{
public:
	//! virtual deconstructor
	CoreExport virtual ~EditSSCB();

	//! Accept an operation
	/*! This operation will be called whenever an operation is finished and the client needs to perform some acceptance operation.
	\param [in]t The current time.
	*/
	virtual void DoAccept(TimeValue t) =0;
	//! Set the pinch parameter
	/*! Set the internal, current soft selection pinch value.
	\param[in] t The current time.
	*/
	//! Set the falloff
	/*! This operation will be called whenever the falloff needs to be set.
	\param[in] t The current time. 
	\param[in] falloff  The soft selection falloff value that should get saved locally.
	*/
	virtual void SetFalloff(TimeValue t,float falloff) =0;	virtual void SetPinch(TimeValue t, float pinch) =0;
	//! Set the bubble paramter
	/*! Set the internal, current soft selection bubble value.
	\param[in] t The current time.
	*/
	virtual void SetBubble(TimeValue t, float bubble) =0;
	//! Get the falloff paramter
	/*! Get the internal, current soft selection falloff value.
	\param[in] t The current time.
	*/
	virtual float GetFalloff(TimeValue t) =0;
	//! Get the pinch paramter
	/*! Get the internal, current soft selection pinch value.
	\param[in] t The current time.
	*/
	virtual float GetPinch(TimeValue t) =0;
	//! Get the bubble paramter
	/*! Get the internal, current soft selection bubble value.
	\param[in] t The current time.
	*/
	virtual float GetBubble(TimeValue t) =0;


};



class EditSSMouseProc;


//! Edit Soft Selection Command Mode for interactively setting soft selection paramters.
/*! This Edit Soft Selection CommandMode is a new command mode that lets the user interactive set a soft selections fallout, pinch and 
bubble paratmers.  To use this class the client needs to implement the virtual base class EditSSCB \see EditSSCB, and then pass that
object in to the constructor.  It is the responsibility of the client to create, manage and remove this command mode.
*/
class EditSSMode : public CommandMode {
private:
	ChangeFGObject		mFGProc;
	EditSSMouseProc*	mpEproc;
	EditSSCB*			mpCB;

public:
	//! Constructor
	/*! Construct the CommandMode
	\param[in] e The EditSSCB virtual base class that needs to get implemented.
	\param[in] rt The ReferenceTarget that's internally used to change the foreground procedure. \see CommandMode::ChangeFG 
	\param[in] i The local interface.
	*/
	CoreExport EditSSMode(EditSSCB* e, ReferenceTarget *rt,IObjParam *i);
	//! Deconstructor
	CoreExport virtual ~EditSSMode();
	
	//! Class Type implementation. \see CommandMode::Class
	int							Class() { return MODIFY_COMMAND; }
	//! ID implementation. \see CommandMode::ID
	int							ID() { return  CID_EDITSOFTSELECTION; }
	//! MouseProc implementation. \see CommandMode::MouseProc
	MouseCallBack				*MouseProc(int *numPoints); 
	//! ChangeFGProc implementation. \see CommandMode::ChangeFGProc
	ChangeForegroundCallback	*ChangeFGProc() { return &mFGProc; }
	//! ChangeFG implementation. \see CommandMode::ChangeFG
	BOOL						ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &mFGProc; }

	//! EnterMode implementation. \see CommandMode::EnterMode
	void EnterMode();

	//! ExitMode implementation. \see CommandMode::ExitMode
	void ExitMode();
};


