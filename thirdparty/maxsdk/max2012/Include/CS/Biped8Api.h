/*********************************************************************
 *<
	FILE: Biped8Api.h

	DESCRIPTION: These are functions that are exported in biped.dlc, updated for Max 8

	CREATED BY:	Susan Amkraut

	HISTORY: Created 18 January 2005

 *>	Copyright (c) 2005 All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\maxheap.h"
#include "BipedApi.h"
#include "IMixer8.h"

/*! \defgroup BipedInterface8 Biped Interface 8
Max 8 Biped export interface
*/
//@{
#define I_BIPMASTER8	0x9167    //!< Biped interface identifier
#define GetBipMaster8Interface(anim) ((IBipMaster8*)(anim)->GetInterface(I_BIPMASTER8))	//!< Gets the Biped interface
//@}

/*! \defgroup BipedSnapshotTypes Biped Snapshot Types
The following are flags sent to describe the type of snapshot a biped copy should store.
*/
//@{
#define SNAP_VIEW		0    //!< Snapshot of the active viewport
#define SNAP_AUTO		1    //!< Snapshot camera position computed automatically to make a close up of the copied parts
#define SNAP_NONE		2    //!< No Snapshot
//@}

/*! \defgroup BipedTCBIKPasteOptions Biped TCB/IK Paste Options
The following are flags used to describe how the TCB and IK parameters are to pasted.
These flags are only used to paste a pose or posture in auto-key mode.
*/
//@{
#define PST_DEF			0    //!< Paste the default TCB / IK values
#define PST_COPY		1    //!< Paste the TCB / IK values which were stored in the copy
#define PST_INTERP		2    //!< Paste an interpolation of the surrounding TCB / IK values
//@}

/*! \defgroup BipedModes Biped Modes
The following are flags which describe new biped modes in Max 8.  They can be sent to the BeginModes and EndModes
functions of IBipMaster, and retrieved from the GetActiveModes function of IBipMaster.
The first three modes and BMODE_BENDLINKS are mutually exculsive.
*/
//@{
#define BMODE_TWISTLINKS	(1<<12)    //!< Twist Links Mode
#define BMODE_TWISTIND		(1<<13)    //!< Twist Individual Mode
#define BMODE_SMOOTHTWIST	(1<<14)    //!< Smooth Twist Mode
#define BMODE_ALL			(BMODE_FIGURE|BMODE_FOOTSTEP|BMODE_MOTIONFLOW| \
							 BMODE_BUFFER|BMODE_RUBBERBAND| \
							 BMODE_BENDLINKS|BMODE_TWISTLINKS|BMODE_TWISTIND|BMODE_SMOOTHTWIST \
							 BMODE_SCALESTRIDE|BMODE_INPLACE|BMODE_MIXER|BMODE_MOVEALL)    //!< All Modes
//@}

class IBipMaster8;

//! \brief A copy within a copy collection.
/*! The copy is either a posture, pose, or track.  
You can find out what it is with the GetCopyType function.
You can get an instance of this class from a copy collection.
*/
class IBipedCopy: public MaxHeapOperators
{
  public:
	//! \brief Destructor
	virtual ~IBipedCopy() {;}

	//! \brief Gets the name of the copy.
	//! \return the name of the copy
	virtual const MCHAR *GetName() const = 0;

	//! \brief Sets the name of the copy.
	//! \pre name must be <= 256 characters
	//! \param[in] name - name of the copy
	//! \return true if the name was set, false if it wasn't set
	virtual bool SetName(const MCHAR *name) = 0;

	//! \brief Gets the type of the copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \return the type of the copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	virtual int GetCopyType() const = 0;
};

//! \brief A collection of copies.
/*! This contains a collection of IBipCopy objects - postures, poses, and tracks.
Access to an IBipCopy object is based on its type and its index in the collection of copy objects.
An instance of ICopyCollection can be gotten from the IBipMaster8 GetCopyCollection and CreateCopyCollection functions.
*/
class ICopyCollection: public MaxHeapOperators
{
  public:
	//! \brief Destructor
	virtual ~ICopyCollection() {;}

	//! \brief Gets the name of the copy collection.
	//! \return the name of the copy
	virtual const MCHAR *GetName() const = 0;

	//! \brief Sets the name of the copy collection.
	//! \pre name must be <= 256 characters
	//! \param[in] name - name of the copy collection
	//! \return true if the name was set, false if it wasn't set
	virtual bool SetName(const MCHAR *name) = 0;

	//! \brief Gets the number of copies of the specified type (posture/pose/track).
	//! \param[in] copyType - type of copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \return the number of copies of the specified type
	virtual int NumCopies(int copyType) = 0;

	//! \brief Gets the copy of the specified type at the specified index
	//! \param[in] copyType - type of copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \param[in] index - the index of the desired copy
	//! \return the copy of the specified type at the specified index, and NULL if the copy was not found
    virtual IBipedCopy *GetCopy(int copyType, int index) = 0;

	//! \brief Gets a copy of the specified type with the specified name
	//! \param[in] copyType - type of copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \param[in] name - the name of the desired copy
	//! \param[out] index - the index of the desired copy
	//! \return the copy of the specified type with the specified name, and NULL if the copy was not found
    virtual IBipedCopy *GetCopy(int copyType, const MCHAR *name, int *index) = 0;

	//! \brief Deletes a copy.
	//! \param[in] copyType - type of copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \param[in] index - the index of the copy to delete
	//! \return true if the copy was deleted, false if the copy was not found
	virtual bool DeleteCopy(int copyType, int index) = 0;

	//! \brief Deletes all copies of a particular type (posture/pose/track) in a collection.
	//! \param[in] copyType - type of copy, one of the following: COPY_POSTURE, COPY_POSE, COPY_TRACK
	//! \return true if the copies were deleted, false if the copy collection was not found
	virtual	void DeleteAllCopies(int copyType) = 0;
};

//! \brief Interface into the biped master controller for Max 8.
/*! This contains functions supporting new biped features in Max 8.  These include
euler versus quaternion rotations, locked com, twist bones, twist poses, bend links functions, and copy collections.
*/
class IBipMaster8: public IBipMaster 
{
	public:

		//! \brief Updates the biped user interface.  
		//! This will update all biped menus in the creation or motion panel, whichever is visible.
		virtual void	RefreshBipedUI() = 0;

		/************ Quaternion / Euler Track Specification **************/
//! \name Quaternion / Euler Track Specification
//@{
		//! \brief Gets whether or not a biped track is euler
		//! \param[in] id - a biped track, as defined in tracks.h 
		//! \return true if if the track is euler, false if it is quaternion
		virtual bool GetEulerActive(const int id) = 0;

		//! \brief Gets the euler order of a biped track
		//! \param[in] id - a biped track, as defined in tracks.h 
		//! \return eulerOrder - an EULERTYPE, as defined in euler.h
		virtual int  GetEulerOrder(const int id) = 0;

		//! \brief Sets a biped track to euler with the specified euler order
		//! \param[in] id - a biped track, as defined in tracks.h. 
		//! Props, fingers, toes, footprints, horizontal, and vertical tracks cannot be set to euler.
		//! \param[in] eulerOrder - any of the EULERTYPEs, as defined euler.h, 
		//! except for EULERTYPE_XYX, EULERTYPE_YZY, and EULERTYPE_ZXZ
		//! \return true if successful, false if not
		virtual bool SetEulerActive(int id, int eulerOrder) = 0;

		//! \brief Sets a biped track to quaternion 
		//! \param[in] id - a biped track, as defined in tracks.h 
		//! \return true if successful, false if not
		virtual bool SetQuaternionActive(int id) = 0;
		
		//! \brief Sets a biped track to the specified euler order.  
		//! The euler order is only relevant if the track is euler or when it becomes euler.
		//! \param[in] id - a biped track, as defined in tracks.h 
		//! \param[in] eulerOrder - any of the EULERTYPEs, as defined in euler.h, 
		//! except for EULERTYPE_XYX, EULERTYPE_YZY, and EULERTYPE_ZXZ
		//! \return true if successful, false if not
		virtual bool SetEulerOrder(int id, int eulerOrder) = 0;
//@}

		/************ Center of Mass Selection ***************/
//! \name Center of Mass Selection
//@{		
		//! \brief Queries the locked state of the com
		//! \return true if the com is locked, false if not
		virtual bool	IsComLocked()=0;

		//! \brief Locks or unlocks the com
		//! \param[in]  lock - true to lock the com, false to unlock the com
		virtual void	LockCom(bool lock)=0;
//@}

		/************ Functions to Manipulate Twist Bones ***************/
//! \name Manipulation of Twist Bones
//@{
		//! \brief Queries the state of Twists for this biped.
		//! \return true if the twists are on, false if the twists are off
		virtual bool	GetTwist() const =0;
		
		//! \brief Enables or disables the twists for this biped.
		//! \pre Biped must be in figure mode or must be active in the biped creation panel.
		//! \param[in] enable - If true, new twists will be added.  If false, all twists will be removed.
		virtual void	SetTwist(bool enable)=0;
		
		//! \brief Gets the twist bias for the specified twist track of this biped.
		//! \param[in] twisttrack - one of the following twist track identifiers: 
		//! TWS_LFARM, TWS_RFARM, TWS_LCALF, TWS_RCALF, TWS_LHORSE, TWS_RHORSE
		//! \return the twist bias, a number between 0.0 and 1.0
		virtual float	GetTwistBias(int twisttrack) const =0;
			
		//! \brief Sets the twist bias for the specified twist track of this biped.
		//! \param[in] twisttrack - one of the following twist track identifiers: 
		//! TWS_LFARM, TWS_RFARM, TWS_LCALF, TWS_RCALF, TWS_LHORSE, TWS_RHORSE
		//! \param[in] bias - the bias, between 0.0 and 1.0
		virtual void	SetTwistBias(int twisttrack, float bias)=0;
//@}

		/************ Functions to Manipulate Twist Poses ***************/
//! \name Twist Poses
//@{
		//! \brief Queries the number of twist poses for the specified limb.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM for arm poses, KEY_LLEG or KEY_RLEG for leg poses
		//! \return the number of arm or leg twist poses
		virtual int  NumTwistPoses(int keytrack)=0;

		//! \brief Creates a new arm or leg twist pose, and appends it the the existing poses.  
		//! The pose will be the current rotation of the upper arm for the arm track, and the thigh for the leg track.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to create an arm pose, and KEY_LLEG or KEY_RLEG to create a leg pose
		//! \return true if the function was successful, false if not
		virtual bool CreateTwistPose(int keytrack)=0;

		//! \brief Sets an existing arm or leg twist pose.  
		//! The pose will be the current rotation of the upper arm for the arm track, and the thigh for the leg track.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set an arm pose, and KEY_LLEG or KEY_RLEG to set a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \return true if the function was successful, false if not
		virtual bool SetTwistPose(int keytrack, int index)=0;

		//! \brief Deletes an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set an arm pose, and KEY_LLEG or KEY_RLEG to set a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \return true if the function was successful, false if not
		virtual bool DeleteTwistPose(int keytrack, int index)=0;

		//! \brief Sets the arm or leg twist poses to the default poses.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set the arm poses, and KEY_LLEG or KEY_RLEG to set the leg poses
		virtual void SetDefaultTwistPoses(int keytrack)=0;

		//! \brief Sets the name of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set an arm pose, and KEY_LLEG or KEY_RLEG to set a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \param[in] name - the name to call the pose
		//! \return true if the function was successful, false if not
		virtual bool SetTwistPoseName(int keytrack, int index, const MCHAR *name)=0;

		//! \brief Sets the twist value of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set an arm pose, and KEY_LLEG or KEY_RLEG to set a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \param[in] twist - the twist value in degrees, between -180 and 180
		//! \return true if the function was successful, false if not
		virtual bool SetTwistPoseTwist(int keytrack, int index, float twist)=0;

		//! \brief Sets the bias value of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to set an arm pose, and KEY_LLEG or KEY_RLEG to set a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \param[in] bias - the bias value, between 0.0 and 1.0
		//! \return true if the function was successful, false if not
		virtual bool SetTwistPoseBias(int keytrack, int index, float bias)=0;

		//! \brief Returns the name of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to get an arm pose, and KEY_LLEG or KEY_RLEG to get a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \return the name of the specified pose
		virtual const MCHAR *GetTwistPoseName(int keytrack, int index)=0;

		//! \brief Returns the twist value of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to get an arm pose, and KEY_LLEG or KEY_RLEG to get a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \return the twist value of the specified twist pose
		virtual float GetTwistPoseTwist(int keytrack, int index)=0;

		//! \brief Returns the bias value of an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to get an arm pose, and KEY_LLEG or KEY_RLEG to get a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \return the bias value of the specified pose
		virtual float GetTwistPoseBias(int keytrack, int index)=0;

		//! \brief Updates the limb to match an arm or leg twist pose.
		//! \param[in] keytrack - KEY_LARM or KEY_RARM to get an arm pose, and KEY_LLEG or KEY_RLEG to get a leg pose
		//! \param[in] index - the index of the pose in the pose list
		//! \param[in] opposite - the twisties of the opposite limb will also be updated if this is true
		virtual void SetLimbToTwistPose(int keytrack, int index, bool opposite)=0;

		//! \brief Loads a figure file without loading the twist poses stored in the file
		//! \param[in] fname - name of figure file
		//! \param[in] redraw - redraw viewports if true
		//! \param[in] msgs - pop up error dialogs if an error occurs
		//! \return true if the function was successful, false if not
		virtual int		LoadFigNoTwists		(const MCHAR *fname, BOOL redraw = false, BOOL msgs = false)=0; 

		//! \brief Loads only the twist poses from a figure file
		//! \param[in] fname - name of figure file
		//! \param[in] redraw - redraw viewports if true
		//! \param[in] msgs - pop up error dialogs if an error occurs
		//! \return true if the function was successful, false if not
		virtual int		LoadFigJustTwists	(const MCHAR *fname, BOOL redraw = false, BOOL msgs = false)=0; 
//@}

		/************ Bend Links Menu Functions, for rotations of spine, neck, tail, and pony tail links ***************/
//! \name Bend Links Menu Functions
//! \brief For spine, neck.tail, and pony tail link rotations
//@{
		//! \brief Zeros out the twist for the specified track of this biped.
		//! \param[in] keytrack - one of the following track identifiers: KEY_SPINE,KEY_NECK,KEY_TAIL,KEY_PONY1,KEY_PONY2
		virtual void	ZeroTwist(int keytrack)=0;

		//! \brief Zeros out all the angles for all the links of the specified track of this biped.
		//! \param[in] keytrack - one of the following track identifiers: KEY_SPINE,KEY_NECK,KEY_TAIL,KEY_PONY1,KEY_PONY2
		virtual void	ZeroAll(int keytrack)=0;

		//! \brief Smooths the twist between the first and last link of the specified track of this biped.
		//! \param[in] keytrack - one of the following track identifiers: KEY_SPINE,KEY_NECK,KEY_TAIL,KEY_PONY1,KEY_PONY2
		//! \param[in] bias - the bias, between 0.0 and 1.0
		virtual void	SmoothTwist(int keytrack, float bias)=0;
//@}

		/************ Copy/Paste Functions which work with Copy Collections ***************/
//! \name Copy/Paste Functions and Copy Collections
//@{
		//! \brief Queries the number of copy collections.
		//! \return the number of copy collections.
		virtual int NumCopyCollections() const = 0;

		//! \brief Gets a copy collection.
		//! \param[in] index - index of desired copy collection
		//! \return the specified copy collection, NULL if the index was out of bounds
		virtual ICopyCollection *GetCopyCollection(int index) const = 0;

		//! \brief Gets a copy collection.
		//! \param[in] name - name of desired copy collection
		//! \param[out] index - index of desired copy collection
		//! \return the specified copy collection, NULL if the index was out of bounds
		virtual ICopyCollection *GetCopyCollection(const MCHAR *name, int *index) const = 0;

		//! \brief Creates a copy collection.
		//! \param[in] colname - name of new copy collection
		//! \return the newly created copy collection
		virtual	ICopyCollection *CreateCopyCollection(const MCHAR *colname) = 0;

		//! \brief Deletes a copy collection.
		//! \param[in] index - index of copy collection to delete
		//! \return true if the collection was deleted, false if the index was out of bounds
		virtual	bool DeleteCopyCollection(int index) = 0;

		//! \brief Deletes all copy collections.
		virtual	void DeleteAllCopyCollections() = 0;

		//! \brief Copies the posture of the specified parts of this biped.
		//! \param[in] ICC - copy collection in which to place the new copy
		//! \param[in] nodes - parts of the biped whose posture will be copied.  
		//! If this is NULL, the posture of the selected parts of the biped will be copied.
		//! \param[in] snapshottype - type of snapshot to store, one of the following: SNAP_VIEW (active viewport), SNAP_AUTO (automatic), SNAP_NONE (none)
		//! \return the new copy, NULL if a copy was not created
		virtual IBipedCopy *CopyBipPosture(ICopyCollection *ICC, INodeTab *nodes, int snapshottype) = 0;

		//! \brief Copies the biped's pose.
		//! \param[in] ICC - copy collection in which to place the new copy
		//! \param[in] snapshottype - type of snapshot to store, one of the following: SNAP_VIEW (active viewport), SNAP_AUTO (automatic), SNAP_NONE (none)
		//! \return the new copy, NULL if a copy was not created
		virtual IBipedCopy *CopyBipPose(ICopyCollection *ICC, int snapshottype) = 0;
		
		//! \brief Copies the animation of the specified parts of this biped.
		//! \param[in] ICC - copy collection in which to place the new copy
		//! \param[in] nodes - parts of the biped whose animation will be copied.  
		//! If this is NULL, the animation of the selected parts of the biped will be copied.
		//! \param[in] snapshottype - type of snapshot to store, one of the following: SNAP_VIEW (active viewport), SNAP_AUTO (automatic), SNAP_NONE (none)
		//! \return the new copy, NULL if a copy was not created
		virtual IBipedCopy *CopyBipTrack(ICopyCollection *ICC, INodeTab *nodes, int snapshottype) = 0;
		
		//! \brief Pastes a posture onto this biped
		//! \param[in] CP - a biped copy of type posture
		//! \param[in] opposite - true to paste opposite, false to paste normal
		//! \param[in] KeyPasteType - method to paste TCB/IK values, one of the following: PST_DEF (default), PST_COPY (copied), PST_INTERP (interpolated).  
		//! KeyPasteType is only relevant if you are in auto key mode
		//! \param[in] Hor - true to paste the horizontal com, false to keep the existing horizontal com
		//! \param[in] Ver - true to paste the vertical com, false to keep the existing vertical com
		//! \param[in] Trn - true to paste the rotation com, false to keep the existing rotation com.
		//! Hor, Ver, Trn are only relevant if you are pasting a posture with a copied com
		//! \param[in] ByVel - true to paste the com by velocity, false to paste the com absolute, relevant for true values of Hor, Ver, Trn
		//! \return true if the paste was successful, false if the paste was unsuccessful
		virtual bool PasteBipPosture(IBipedCopy *CP, bool opposite, int KeyPasteType, bool Hor, bool Ver, bool Trn, bool ByVel) = 0;
				
		//! \brief Pastes a pose onto this biped
		//! \param[in] CP - a biped copy of type pose
		//! \param[in] opposite - true to paste opposite, false to paste normal
		//! \param[in] KeyPasteType - method to paste TCB/IK values, one of the following: PST_DEF (default), PST_COPY (copied), PST_INTERP (interpolated).  
		//! KeyPasteType is only relevant if you are in auto key mode.
		//! \param[in] Hor - true to paste the horizontal com, false to keep the existing horizontal com
		//! \param[in] Ver - true to paste the vertical com, false to keep the existing vertical com
		//! \param[in] Trn - true to paste the rotation com, false to keep the existing rotation com
		//! \param[in] ByVel - true to paste the com by velocity, false to paste the com absolute, relevant for true values of Hor, Ver, Trn
		//! \return true if the paste was successful, false if the paste was unsuccessful
		virtual bool PasteBipPose(IBipedCopy *CP, bool opposite, int KeyPasteType, bool Hor, bool Ver, bool Trn, bool ByVel) = 0;
				
		//! \brief Pastes a track onto this biped
		//! \param[in] CP - a biped copy of type track
		//! \param[in] opposite - true to paste opposite, false to paste normal
		//! \param[in] Hor - true to paste the horizontal com, false to keep the existing horizontal com
		//! \param[in] Ver - true to paste the vertical com, false to keep the existing vertical com
		//! \param[in] Trn - true to paste the rotation com, false to keep the existing rotation com
		//! \return true if the paste was successful, false if the paste was unsuccessful
		virtual bool PasteBipTrack(IBipedCopy *CP, bool opposite, bool Hor, bool Ver, bool Trn) = 0;
//@}
		//! \brief Get the new mixer interface for max 8 that supports max objects
		//! \return The IMixer8 interface
		//! \see IMixer8
		virtual IMixer8 * GetMixer8() =0;
};

