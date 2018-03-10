/*==============================================================================
Copyright 2008 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
or which otherwise accompanies this software in either electronic or hard copy form.   
*****************************/
// DESCRIPTION: Interface to Locked Tracks
// AUTHOR: Michael Zyracki created 2008
//***************************************************************************/
#pragma once

#include "ifnpub.h"
#include "control.h"
#include "AnimatableInterfaceIDs.h"

//! \brief The unique interface ID for the Locked Tracks Manager
#define ILOCKEDTRACKSMAN_INTERFACE Interface_ID(0x30384fe4, 0x3d657ac6)

/*! \brief Locked Track Manager
This class provides an interface for managing and setting locks on acceptable tracks.\n\n
To get a pointer to this class simply call 
\code
static_cast<ILockedTracksMan*>( GetCOREInterface(ILOCKEDTRACKSMAN_INTERFACE ) );
\endcode
*/
class ILockedTracksMan: public FPStaticInterface
{
public:

	//! \brief Function to see if a particular track is lockable.
	//! This funcion is used to see if a particular track may get locked and ulocked.
	//! \param[in] anim The track that we are checking to see if it's lockable.
	//! \param[in] client The parent of the track that we are testing to see if it's lockable.
	//! \param[in] subNum The subanim number of the anim related to it's client.
	//! \return Returns true if a track can get locked and unlocked, false if it can't get locked or unlocked.
	virtual bool IsLockable(ReferenceTarget *anim,ReferenceTarget *client, int subNum) =0;

	//! \brief Function to see if a particular track is locked or unlocked.
	//! This function is used to see if a track is locked or unlocked.
	//! \param[in] anim The track that we are checking to see if it's locked
	//! \param[in] client The parent of the track that we are testing to see if it's locked.
	//! \param[in] subNum The subanim number of the anim related to it's client.
	//! \param[in] checkOverride  If true we check the overrides first before testing to see if the item is locked,
	//! if false then we don't check the override state and rather just check the state of the locked item by itself.
	//! \return Returns true if a track is locked, false if it's unlocked or if it's unlockable. \see IsLockable
	virtual bool GetLocked(ReferenceTarget *anim, ReferenceTarget *client, int subNum,bool checkOverride=true) =0;

	//! \name Setting Locks
	//@{
	//! \brief Function to set or unset a lock on a list of possible tracks.
	//! This function is used to lock or unlock a list of possible tracks. The tracks are represented by a pointer to
	//! the track, a pointer to their parent, and it's parent sub anim number.  You can specify if you want to propagate the
	//! locking or unlocking to the tracks children.
	//! \param[in] val If true we lock, if false we unlock.
	//! \param[in] anims The list of tracks that you want to lock or unlock
	//! \param[in] clients The list of the tracks parents.
	//! \param[in] subNums The list of the subAnim id's for each of the track for each parent.
	//! \param[in] lockSubAnims If true then we also lock or unlock the tracks subanims, if false then we don't.
	virtual void SetLocks(bool val,Tab<ReferenceTarget*> &anims, Tab<ReferenceTarget*> &clients,Tab<int> &subNums,
							bool lockSubAnims)=0;

	//! \brief Function to toggle a lock on a list of possible tracks.
	//! This function is used to toggle a lock on a list of possible tracks. The tracks are represented by a pointer to
	//! the track , a pointer to their parent, and it's parent sub anim number.  You can specify if you want to propagate the
	//! locking or unlocking to the tracks children.
	//! \param[in] anims The list of tracks that you want to toggle the lock on.
	//! \param[in] clients The list of the tracks parents.
	//! \param[in] subNums The list of the subAnim id's for each of the track for each parent.
	//! \param[in] lockSubAnims If true then we also toggle the locks on the tracks sub anims, if false we don't.
	virtual void ToggleLocks(Tab<ReferenceTarget*> &anims, Tab<ReferenceTarget*> &clients,Tab<int> &subNums,
							bool lockSubAnims)=0;
	//@}
	


	//! \name Unlocking Overrides
	//@{
	//! \brief Function to unlock all locks. Note will NOT unlock a locked item in an imported container.
	//! \param[in] val The value of the current state of the unlock override. If true then all locked items,
	//! not in an imported container, will become unlocked.
	virtual void SetUnLockOverride(bool val) =0;

	//! \brief Function to get the state of the unlock override.
	//! \return  Returns the value fo the current state of the unlock override. If true then all locked items,
	//! not in an imported container, are unlocked.
	virtual bool GetUnLockOverride() const =0;
	

	//! \brief Function to override all locked tracks, even those in containers. 
	//! This function will push an unlock override state that will let you override and unlock all tracks,
	//! including those that locked in a container.  Useful usually when you want
	//! to have a script, like a script controller, still be able to drive locked tracks.  Note that you need to
	//! call PopUberUnLockOverride to when you are done with this override and it should be used in a limited, local manner.
	virtual void PushUberUnLockOverride()=0;
	//! \brief Function to pop the uber unlock override state.
	//! This function will pop an unlock override state that will let you override and unlock all tracks,
	//! including those that locked in a container.  This function needs to be called at some point after every
	//! PushUberUnLockOverride function call. If called before PushUberUnLockOverride an error may occur.
	virtual void PopUberUnLockOverride()=0;
	//! \brief Function to get the state of the uber unlock override.
	//! This function will return the value of how many existing pushes of the uber unlock override state exist. So,
	//! if this value is greater or equal to one then we have an active uber unlock override occuring, if the value is zero
	//! then we don't have this override, if this value is less than zero, then we have an error since a PopUberUnLockOverride(),
	//! was called without first calling PushUberUnLockOverride.
	virtual int GetUberUnLockOverride()const =0;

	//@}

	//! \name Convenience Functions
	//@{
	//! \brief Function to find all locked or unlocked anims on a node
	//! \param[in] locked If true we find locked items, if false we find unlocked items. If an item is not lockable, it is not included
	//! with either option.
	//! \param[in] checkOverride  If true we check the overrides first before testing to see if the item is locked,
	//! \param[in] refTarget The reference target we will find the anims on.
	//! \param[out] anims The list of anims that are locked or unlocked.
	//! \param[out] clients The parents of the anims that are locked or unlocked.
	//! \param[out] subNums The subanim index of the anims that are locked or unlocked
	virtual void FindLockedAnims(bool locked,bool checkOverride, ReferenceTarget *refTarget, Tab<ReferenceTarget*> &anims, Tab<ReferenceTarget*> &clients, Tab<int> &subnums) =0;

	//! \brief Function to test whether or not a particular anim is overriden unlocked or not
	//! \param[in] anim The anim we are testing to see if it's overriden unlocked or not.
	//! \return  If true the item is overriden unlocked and a locked item should now return false when queried for it's locked state.
	virtual bool IsAnimOverrideUnlocked(ReferenceTarget *anim) =0;
	//@}

};

//! \brief An interface to a track that can get locked.
//! Your animatable class should inherit from this class if it wants to support itself getting locked.  Remember
//! only items which can get locked can support local edits with containers.
//! To get from an existing object, call GetLockedTrackInterface(anim) which is defined AnimatableInterfaceIDs.h
class ILockedTrack: public MaxHeapOperators 
{
public:
	//! \brief deconstructor
	virtual ~ILockedTrack(){};
	//! \brief Function to lock or unlock this track
	//! This function will be called to lock or unlock this item. The implementator of this function is responsible
	//! for setting and saving the locked state of the track based upon the value of the passed in flag. Also it's the responsible
	//! of the implementator to lock it's sub anims if the flag is set to also lock it's subanims.  The system itself is responsible
	//! to making sure all of it's parents are unlocked when it gets unlocked.
	//! \param[in] val If true the track gets locked, if false the track gets unlocked
	//! \param[in] anim A pointer to the reference target that contains this interface.
	//! \param[in] client A pointer to the parent of this interface.
	//! \param[in] subNum The subanim index of this track.
	virtual void SetLocked(bool val, bool lockSubanims,ReferenceTarget* anim,ReferenceTarget* client, int subNum)=0;
	//! \brief Return whether or not the track is locked.
	//! \param[in] checkOverride  If true we check the overrides first before testing to see if the track is locked,
	//! if false then we don't check the override state and rather just check the state of the locked item by itself.
	//! \return Returns true if the track is locked, false if it is not. Also take note that this track takes into account
	//! whether or not any overrides are active and if it's inside a container or not. \see ILockedTracksMan.
	virtual bool GetLocked(bool checkOverride = true)const =0;

	//! \brief Returns whether or not the object is a component
	//!  Returns whether or not an object is a component. A component is an object that when locked gets replaced by a wrapper object that hides
	//! the existing object inside it.  Lockable Modifiers and Materials may be examples of components. Most basic 'tracks' and controllers
	//! aren't componenets. Thus the default returns false.
	//! \return return true if a component false otherwise
	virtual bool IsComponent(){return false;}

};


//! Internal class, forward definition.
class ILockedTrackImpRestore;

//*! \sa  Class ILockedTrack\n\n
//! \brief This class provides an implementation of ILockedTrack base class, including a local variable that's used for the
//! persisting the lock.
class ILockedTrackImp: public ILockedTrack
{
	friend class ILockedTrackImpRestore;
public:
	//! \brief Constructor
	CoreExport ILockedTrackImp();
	//! \brief Implementation of SetLocked. See ILockedTrack::SetLocked
	CoreExport void SetLocked(bool val, bool effectChildren,ReferenceTarget* anim,ReferenceTarget* client, int SubNum);
	//! \brief Implementation of GetLocked. See ILockedTrack::GetLocked
	CoreExport bool GetLocked(bool checkOverride = true)const; 

protected:
	//! \brief Member variable for storing the current locked state.
	bool mLocked;
};

//! \brief An interface to an object who is a client that has subanims that may get locked.
//! Your animatable class should inherit from this class if it wants to support subanims, that don't return an interface
//! when GetLockedTrackInterface(anim) is called, but can still get locked.  In particularly, IParamBlock and IParamBlock2 objects will
//! return this interface
//! since they will control the locking and unlocking of their parameter entries.
//! To get from an existing object, call GetLockedTrackClientInterface(anim) which is defined AnimatableInterfaceIDs.h
class ILockedTrackClient: public MaxHeapOperators 
{
public:
	//! \brief destructor
	virtual ~ILockedTrackClient(){};
	//! \brief Function to lock or unlock this track's subanim
	//! This function will be called to lock or unlock this clients subanim . The implementer of this function is responsible
	//! for setting and saving the locked state of the subanim track based upon the value of the passed in flag. Also it's the responsible
	//! of the implementer to lock it's subanim subanims if the flag is set to also lock it's subanims.  The system itself is responsible
	//! to making sure all of it's parents are unlocked when it gets unlocked also.
	//! \param[in] val If true the subanim track gets locked, if false the track gets unlocked
	//! \param[in] lockSubanims If true then also lock the children of the anim that you are locking. Usually this is not needed since the 
	//! anim is almost always a leaf.
	//! \param[in] anim A pointer to the reference target that contains this interface.
	//! \param[in] client A pointer to the reference target of this interface. It's the client that contains the subanim we will lock.
	//! \param[in] subNum The subanim index of this track.
	virtual void SetLocked(bool val, bool lockSubanims, ReferenceTarget* anim, ReferenceTarget* client, int subNum)=0;
	//! \brief Return whether or not one if it's subanim tracks are locked.
	//! \param[in] subNum  This is the SubAnim number for the track we want to check to see if it's locked.
	//! \param[in] checkOverride  If true we check the overrides first before testing to see if the track is locked,
	//! if false then we don't check the override state and rather just check the state of the locked item by itself.
	//! \return Returns true if the subanim track is locked, false if it is not. Also take note that this track takes into account
	//! whether or not any overrides are active and if it's inside a container or not. \see ILockedTracksMan.
	virtual bool GetLocked(int subNum, bool checkOverride = true)const =0;

	//! \brief Return whether or not one if its subanim tracks or lockable or not.
	//! \param[in] subNum  This is the SubAnim number for the track we want to check to see if it's lockable or not.
	//! \return Returns true if the subanim track is lockable false if it is not.
	virtual bool IsLockable(int subNum)const =0;
};

/*! \sa  Class StdControl, Class ILockedTrackImp.
This class represents the combination of a StdControl class with the ILockedTrackImp class.
You can use this class to implement locked track functionality over a StdControl class.
*/
class LockableStdControl: public StdControl, public ILockedTrackImp
{
public:

	//! \brief Implementation of GetInterface for locked tracks. See Animatable::IsReplaceable
	void* GetInterface(ULONG id)
	{
		switch (id) {
			case I_LOCKED:
					return (ILockedTrackImp*) this;
			}
		return StdControl::GetInterface(id);
	}
	//! \brief Implementation of IsReplaceable for locked tracks. See Control::IsReplaceable
	BOOL IsReplaceable() {return !GetLocked();}  

	//! \brief Implementation of CanApplyEaseMultCurves for locked tracks. See Control::IsReplaceable
	BOOL CanApplyEaseMultCurves(){return !GetLocked();}
};

/*! \sa  Class Control, Class ILockedTrackImp.\n\n
This class represents the combiniation of a Control class with the ILockedTrackImp class.
You can use this class to implement locked track functionality over a Control class.
*/
class LockableControl : public Control, public ILockedTrackImp
{
public:
	

	//! \brief Implementation of GetInterface for locked tracks. See Animatable::IsReplaceable
	void* GetInterface(ULONG id)
	{
		switch (id) {
			case I_LOCKED:
					return (ILockedTrackImp*) this;
			}
		return Control::GetInterface(id);
	}
	//! \brief Implementation of IsReplaceable for locked tracks. See Control::IsReplaceable
	BOOL IsReplaceable() {return !GetLocked();} 

	//! \brief Implementation of CanApplyEaseMultCurves for locked tracks. See Control::IsReplaceable
	BOOL CanApplyEaseMultCurves(){return !GetLocked();}
};


/*!  Instructions on how to make a controller lockable. Also please look at the samples in maxsdk\samples\controllers since they
	have been made lockable.

1)  Include the ILockedTracksImp.h header file.
2)  Have your controller derive publically from the default ILockedTrackImp locked track implementation, or if possible from either
the StdControlLocked implementation that includes both StdControl and ILockedTrackImp, or the ControlLocked implementation, that
includes Control and ILockedTRackImp.  These implementations contains all of the logic needed to determine whether or not an item
is really locked, and handles the default mechanism for the unlocking and locking of parent and child subanims.  This requires that
you link with core.lib, which you probably already are.  When possible use either StdControlLocked or ControlLocked since they already
contain default implementations of functions needed to properly lock your controller.  Make sure though that if you override these
default implementations of GetInterface, IsReplaceable, and CanApplyEaseMultCurves that you keep the same logic.
3)  Make sure the void* Animatable::GetInterface(ULONG id) function is defined since it's where the locked track interface resides.
At a minimum you need to implement,
\code
void* GetInterface(ULONG id)
{
	switch (id) {
		case I_LOCKED:
				return (ILockedTrackImp*) this;
		}
	return Control::GetInterface(id);
}
\endcode
4)  If your controller is a leaf controller, you need to disable of appending of ease and mult curves to your controller. To do this you'll
need to modify the CanApplyEaseMultCurves to return false when the controller is locked.  If it's not a leaf controller, there is no need to
worry about this function since you can't apply eases or mult controllers to non-leaf controllers.  Sample Implementation is
BOOL CanApplyEaseMultCurves(){return !GetLocked();}
5)  When a controller is locked it cannot be replaced, so you will need to modify the IsReplaceable() function. A sample implementation is
BOOL IsReplaceable() {return !GetLocked();}  
6) Make sure that the ILoad(ILoad*) and ISave(ISave*) functions are implemented and that you save and load the inherited mLocked parameter
from ILockedTrackImp. Here's a sample implementation,
\code
#define LOCK_CHUNK		0x2535  //the lock value
IOResult MasterPointControlImp::Save(ISave *isave)
{
	Control::Save(isave); // save ORT

	ULONG nb;
	int on = (mLocked==true) ? 1 :0;
	isave->BeginChunk(LOCK_CHUNK);
	isave->Write(&on,sizeof(on),&nb);	
	isave->EndChunk();	

	return IO_OK;
}

IOResult MasterPointControlImp::Load(ILoad *iload)
{
	ULONG nb;
	IOResult res;

	res = Control::Load(iload); // Load ORT
	if (res!=IO_OK)  return res;

	while (IO_OK==(res=iload->OpenChunk()))
	{
		ULONG curID = iload->CurChunkID();
		if (curID==LOCK_CHUNK)
		{
			int on;
			res=iload->Read(&on,sizeof(on),&nb);
			if(on)
				mLocked = true;
			else
				mLocked = false;
		}
		iload->CloseChunk();
		if (res!=IO_OK)  return res;
	};
	return IO_OK;	
}
\endcode
7) When you do a Clone(RemapDir &remap), make sure to clone the value of mLocked and when a Copy(Control *from) occurs make sure copy the
mLocked Value if you do copy that controller. 
8) To reiterate, when you save and load, and clone, and if possible, when you copy, explicitly use the mLocked variable, if possible,
since you are explicitly setting or getting that parameter.  For other functions, if you want to see if you are locked use the
ILockedTrackImp::GetLocked() function since that one will also use any active overrides or container states to determine if you are truly locked. 
9) Inside AssignController(Animatable *control,int subAnim) you need to check both to see if you are locked, in which case you don't
assign the controller, and the controller that's getting assigned over isn't locked also. You may also want to modify CanAssignController
but that function is not used reliably to determine if a controller may get assigned. The key thing needed is to modify AssignController.
10) You need to make sure you lock the functions that bring up UI, which are BeginEditParams and EditTrackParams.  You may need to modify
you EndEditParams function in case it always expects BeginEditParams to allocate certain data.   If you do normally allow the editing of
track params you should have the TrackParamsType function TrackParamsType, return TRACKPARAMS_NONE when the track is locked.
11) In addition to the Animatable functions mentioned above, other Animatable functions that you should lock are those that modify the
keys or other information in your controller, such as CopyKeysFromTime,DeleteKeyAtTime,DeleteKeys, DeleteTime, ReverseTime, ScaleTime,
InsertTime, MapKeys ,AddNewKey, MoveKeys, ScaleKeyValues,CreateLockKey, SetSelKeyCoordsExpr, CommitSetKeyBuffer,  RevertSetKeyBuffer, etc..
12) You should also lock certain schematic view functions that let you change link, such as SvCanConcludeLink,and  SVLinkChild 
13) You usually don't have to lock functions that set selection (SetKeyByIndex, etc..) or flag keys (FlagKey) since in most
circumstances the selection or flag state of a key doesn't change how that controller computes it's value.  In general only lock operations
that will have an effect when ::GetValue is called.
14) The key Control functions that you need to lock include, of course, SetValue, any function that set's a sub-controller, like
SetPositionController, or SetRollController. 
15) Control functions that should never get locked include GetValue of course, ChangeParents since you still want to have parents get
changed possibly.
16) For most controllers, the only IK function that needs to get locked is PasteIKParams. The other functions are needed to be left
unlocked or IK may not work.
17) If the controller inherits from IKeyControl, then some of its functions also need to get locked. They include SetKey and AppendKey.
Note that GetKey returns a pointer to the key so in theory the client may modify it you can't lock it since that's the only way the
client can examine the contents of the key.
18) For PRS controllers, lock the setting of inheritance flags in  SetInherintanceFlags.   Don't lock ChangeParents, since when a parent
changes you will want the keys to modify if needed.  Also you may need to unlock sub controllers, via the
ILockedTracksMan::PushUberUnLockOverride, and I when changing parents before calling SetValue on them. 
19) If you need to unlock something temporarily the best way to do it so the use the ILockedTracksMan::PushUberUnLockOverride
function to set up an override that disables all locks, and then the ILockedTracksMan::PopUberUnLockOverride function to revert the
override unlock state. This is safer than individual unlocking the controller since if a controller is in an imported container, it can't
get unlocked via the other SDK functions.
20) We currently don't lock exposed SDK/MXS functions that may set internal values that are normally locked.  Usually this isn't a problem
since we lock out the default UI.   

*/

