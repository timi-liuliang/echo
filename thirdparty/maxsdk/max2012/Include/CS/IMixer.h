/**********************************************************************
 *<
	FILE: IMixer.h

	DESCRIPTION:  Mixer classes

	CREATED BY: Susan Amkraut

	HISTORY: created summer 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

/**********************************************************************
UPDATING THE MIXER:

Many of the mixer functions cause changes to the raw mix .
If you call a function or functions which would change the raw mix, 
then in order to see biped perform the updated raw mix, you must call:
IMixer::InvalidateRawMix();

If you call a function which changes the arrangement of clips in a
transition track, then to update the display and computation of that 
track, you may have to call:  
IMXtrack::ComputeTransClips();
It's possible this may have been called for you, depending on what you did.  
But if you encounter any incomplete updates, call this.

Most of the mixer SDK works the same way as the mixer user interface.  
There are a few exceptions.  The user interface does not allow the user to overlap clips.  
But some SDK functions make it possible to overlap clips.  This is not advisable.  
All mixer SDK functions take into account the snap to frames variable 
as well as the lock transitions variable, both settable in the SDK.  
They do not, however, take into account the snap to clips variable, 
both for technical reasons and because this is more of a user interface feature.
************************************************************************/


#pragma once

#include "..\maxheap.h"
#include "BipExp.h"
#include "Tracks.h"
#include "CSConstants.h"
#include "..\assetmanagement\AssetUser.h"

// forward definition
class IMixer;

class IMXclip: public MaxHeapOperators
{
  public:
		virtual ~IMXclip() {;}
    //**************** timing ***********************
    // The original bounds are the time boundaries of the file loaded into the clip.
    // These bounds never change unless a new file is loaded into the clip.
    virtual void GetOriginalBounds(int *orgstart, int *orgend)=0;
    //
    // The trimmed bounds are the trimmed boundaries of the clip.
    // They are the same as or a subregion of the original bounds.
    // These values do not change when you scale a clip.
    // These boundaries are visible in the mixer when you are in trim mode.  
    virtual void GetTrimBounds(int *trimstart, int *trimend)=0;
    // This also trims the weight curve and time warp appropriately
    // trimstart and trimend must be within the original bounds
    virtual void SetClipTrim(int trimstart, int trimend)=0;
    //
    // The global bounds indicate where the trimmed bounds of the clip are mapped globally.
    // The clip's scale is defined by the relationship between the trimmed interval (trimend - trimstart)
    // and the global interval (globend - globstart).  If those intervals are equal, the clip's scale is 1.
    virtual void GetGlobalBounds(int *globstart, int *globend)=0;
    // globstart must be less than globend.  
    // If not, these return false and do nothing.  Otherwise these return true.
    // Setting these is like pulling one edge of the clip.  It actually scales the clip.
    // This also scales the weight curve, time warp, and the in/out points appropriately
    virtual BOOL SetClipGlobalStart(int globstart)=0;
    virtual BOOL SetClipGlobalEnd(int globend)=0;
    //
    virtual void MoveClip(int timeInc)=0;
    //
    // These also scale the weight curve, time warp, and the in/out points appropriately
    virtual void ScaleClip(float scale)=0;
    virtual void SetClipScale(float scale)=0;

    //**************** timing conversion ***********************
    // The following functions are useful for converting between various time spaces described below:
    //
    // local: a time between trimstart and trimend, inclusive
    //
    // scaled local: a time between 0 and the length of the scaled clip, which can be found this way:
    // GetGlobalBounds(&g1,&g2);  int length = g2 - g1;
    //
    // global: a global time value
    //
    virtual float GetClipScale()=0;
    virtual int LocalToScaledLocal(int t)=0;
    virtual int ScaledLocalToLocal(int t)=0;
    virtual int LocalToGlobal(int t)=0;
    virtual int GlobalToLocal(int t)=0;
    virtual int GlobalToScaledLocal(int t)=0;
    virtual int ScaledLocalToGlobal(int t)=0;
    virtual int GlobalInpoint()=0;
    virtual int GlobalOutpoint()=0;

    //**************** transitions ***********************
    // Transition information is stored in each clip, whether it is currently a valid clip in a transition track or not.
    // A clip stores mostly information for the transition from itself to the next clip.
    // It also stores some information for the transition from the previous clip to itself.
    //
    // Inpoints and outpoints are stored and set in the clip's scaled local time.
    //
    // Preserve Height is stored inside each track, not inside individual clips.
    //
    // The best way to manipulate transition values in a transition track is to loop through the clips
    // in sequential transition order by using the following IMXtrack functions described in IMXtrack:
    // ComputeTransClips()
    // NumTransClips()
    // GetTransClip(int index)
    //
    // The inpoint must be greater than the outpoint.
    // The inpoint must be less than the scaled local trimmed end.
    // If you try to set the inpoint out of bounds, it will not be set and the function will return false.
    // Otherwise, the inpoint will be set and the function will return true.
    virtual int GetNextTransitionInpoint()=0;
    virtual BOOL SetNextTransitionInpoint(int inpt)=0;
    //
    virtual float GetNextTransitionAngle()=0;
    virtual void SetNextTransitionAngle(float angle)=0;
    //
    // When setting the focus, if the value sent in is acceptable (FOCUS_AUTO, FOCUS_COM, FOCUS_LFOOT, FOCUS_RFOOT) 
    // then the focus will be set, and the function will return true.
    // Otherwise, the focus will not be set, and the function will return false.
    virtual int  GetNextTransitionFocus()=0;  // returns one of the following: FOCUS_AUTO, FOCUS_COM, FOCUS_LFOOT, FOCUS_RFOOT, FOCUS_BFEET
    virtual BOOL SetNextTransitionFocus(int focus)=0;
    //
    virtual BOOL IsNextTransitionRolling()=0;
    virtual void SetNextTransitionRolling()=0;
    virtual void SetNextTransitionFixed()=0;
    //
    // Each ease value, and the sum of the ease values, must be between 0.0 and 1.0, inclusive.
    // If not, the set function will not set the values and it will return false.
    virtual void GetNextTransitionEase(float *easeIn, float *easeOut)=0;
    virtual BOOL SetNextTransitionEase(float easeIn, float easeOut)=0;
    //
    // Inpoints and outpoints are stored and set in the clip's scaled local time.
    // The outpoint must be less than the inpoint.
    // The outpoint must be greater than the scaled local trimmed start.
    // If you try to set an outpoint out of bounds, it will not be set and the function will return false.
    // Otherwise, the outpoint will be set and the function will return true.
    virtual int  GetPrevTransitionOutpoint()=0;
    virtual BOOL SetPrevTransitionOutpoint(int outpt)=0;
    //
    virtual BOOL IsPrevTransitionRolling()=0;
    virtual void SetPrevTransitionRolling()=0;
    virtual void SetPrevTransitionFixed()=0;

    //***************** clip animation file **************************
    virtual const MaxSDK::AssetManagement::AssetUser & GetFile()=0;
    // LoadOption must be one of the following: RE_SOURCE_CLIP_ONLY, RE_SOURCE_INSTANCES, RE_SOURCE_FILEGROUP
    // RE_SOURCE_CLIP_ONLY: the file will be loaded into just this single clip
    // RE_SOURCE_INSTANCES: the file will be loaded into this clip and all instances of this clip
    // RE_SOURCE_FILEGROUP: the file will be loaded into this clip and all instances and adaptations of this clip
    virtual BOOL LoadFile(int LoadOption, const MCHAR *fname, BOOL ZeroFootHgt)=0;
	
    //***************** weight curve **************************
	// The weight curve for a clip is only relevant if the clip is in a layer track
	// All time values are in the clip's scaled local time (see note above about the meaning of scaled local time)
    virtual int NumWeights()=0;
	virtual float GetWeight(int i)=0; // returns weight at index i, if i is out of bounds, returns 0.0
	virtual int GetWeightTime(int i)=0; // returns time at index i, if i is out of bounds, returns 0
	virtual BOOL DeleteWeight(int i)=0; // returns false if i is out of bounds
	virtual BOOL SetWeight(int i, float w)=0; // returns false if i is out of bounds, or weight is not between 0.0 and 1.0
    virtual BOOL SetWeightTime(int i, int t)=0; // returns false if i is out of bounds or time is not inbetween prevoius and next time

	// If t is at a key in the weight curve, this returns the weight at that key.
	// Otherwise, it returns the interpolated weight at time t.
	// If there are no weights, this returns 1.0.
    virtual float GetWeightAtTime(int t)=0;

	// If there is a key at time t on the weight curve, this will reset the weight at that key.
	// If not, this will create a new key at time t with weight w.
	// Be sure that t is within the scaled local bounds of the clip.
    virtual int SetWeightAtTime(int t, float w)=0;

	//***************** time warp **************************
	// All time values are in the clip's scaled local time (see note above about the meaning of scaled local time)
	virtual BOOL IsTimeWarpActive()=0;
	virtual BOOL ActivateTimeWarp()=0;
	virtual BOOL DeactivateTimeWarp()=0;
	// Removes all existing time warps, activates time warp,
	// and initializes with one warp at the begining and one at the end of the clip
	virtual BOOL InitializeTimeWarp()=0;
	virtual int NumTimeWarps()=0;
	virtual TimeValue GetTwOrgTime(int i)=0; // returns original time at index i, if i is out of bounds, returns 0
	virtual TimeValue GetTwWarpTime(int i)=0;// returns warped time at index i, if i is out of bounds, returns 0
	virtual TimeValue GetOrgTimeAtWarpedTime(TimeValue t)=0; // returns what original time has been warped to this time
	                                             // if t is out of bounds of the warped time, returns 0
	virtual TimeValue GetWarpedTimeAtOrgTime(TimeValue t)=0; // returns what this original time has been warped to
	                                             // if t is out of bounds of the original time, returns 0
	virtual BOOL DeleteTw(int i)=0; // returns false if i is out of bounds
	virtual BOOL SetTwOrgTime(int i, TimeValue t)=0;// returns false if i is out of bounds or t is not inbetween 
	                                          // the two original times of the warp indices surrounding i
	virtual BOOL SetTwWarpTime(int i, TimeValue t)=0;// returns false if i is out of bounds or t is not inbetween 
	                                          // the two warped times of the warp indices surrounding i
	// if t is at an existing warp key's original time value, or t is out of bounds of the clip's scaled local bounds,
	// then this returns false.  Otherwise it sets a new time warp key, computing the warped time for this key such
	// that the flow is continuous
	virtual BOOL InsertWarpAtOrgTime(TimeValue t)=0;
};

class IMXtrack: public MaxHeapOperators
{
  public:
		virtual ~IMXtrack() {;}
    virtual int NumClips(int row)=0;
    virtual IMXclip *GetClip(int index, int row = BOT_ROW)=0; // For layer tracks, row should always be BOT_ROW
    virtual int GetTrackType()=0;                             // returns LAYERTRACK or TRANSTRACK
    virtual void SetTrackType(int toType)=0;                  // must be set to LAYERTRACK or TRANSTRACK
    virtual BOOL GetMute()=0;
    virtual BOOL SetMute(BOOL val)=0;
    virtual BOOL GetSolo()=0;
    virtual BOOL SetSolo(BOOL val)=0;
    virtual void GetInterval(Interval& iv)=0;                 // gets the total time interval of the track, including all clips
    virtual void Clear()=0;
    // The specified bip file will be loaded into the new clip, and the reservoir, using the ZeroFootHeight parameter.  
    // For layer tracks, the start of the new clip will be appended interval time from the end of the last clip in the track.  
    // The interval variable must be >= 0.   
    // For transition tracks, the interval parameter is ignored.  The start of the new clip will be at 
    // the inpoint of the last clip in the track, and it will be on the opposite row of the last clip.  
    // It will start later if it collides with another clip.  For both layer and transition tracks, 
    // if there are no clips in the track, the new clip will start at frame 0.  This returns false if 
    // the specified file could not be loaded.  Otherwise, it returns true.
    virtual BOOL AppendClip(const MCHAR *fname, BOOL ZeroFootHeight = true, int interval = 5)=0;

    /******************** transition track functions (not for layer tracks) **************************/
    // If any of these functions are called for layer tracks, they will return false and do nothing.
    // The track must be a transition track.
    //
    virtual BOOL IsHeightPreservedDuringTransitions()=0;
    virtual BOOL SetTransPreserveHeight(BOOL val)=0;
    // PreferredTransLength and the search ranges must be positive values.  They are in frames.  
    // If they are negative values, the optimization will not be performed and the function will return false.  
    // Otherwise, the function will return true/false describing whether or not it was successful.  
    // If optimization requires clips to be trimmed to avoid visual overlap, then they will be trimmed.
    virtual BOOL OptimizeTransitions(int PreferredTransLength, BOOL SearchEntireClip, int SearchRangeBefore, int  SearchRangeAfter)=0;
    // See notes above.
    // The ClipIndex is an index into the tracks's trans clips.
    // If the ClipIndex is out of range of the trans clips, the optimization will not be performed and the function will return false.
    virtual BOOL OptimizeClipsNextTransition(int PreferredTransLength, BOOL SearchEntireClip, int SearchRangeBefore, int  SearchRangeAfter, int ClipIndex)=0;

    // The following three functions provide a way of looping through the clips in a transition track in 
    // sequential order, based on how they transition from one to another, regardless of what row they are in.
    // A greyed out clip will not be included in the transclips, since it is not part of the series of transitions.
    // If you change ANYTHING in a transition track (clip timiing, transition timing or parameters), 
    // then you must call ComputeTransClips before looping through the trans clips
    virtual void ComputeTransClips()=0;
    virtual int NumTransClips()=0;
    virtual IMXclip *GetTransClip(int index)=0;	

    //***************** weight curve **************************
	// The weight curve for a track is only relevant if the track is a transition track
    virtual int NumWeights()=0;
	virtual float GetWeight(int i)=0; // returns weight at index i, if i is out of bounds, returns 0.0
	virtual int GetWeightTime(int i)=0; // returns time at index i, if i is out of bounds, returns 0
	virtual BOOL DeleteWeight(int i)=0; // returns false if i is out of bounds
	virtual BOOL SetWeight(int i, float w)=0; // returns false if i is out of bound, or weight is not between 0.0 and 1.0
    virtual BOOL SetWeightTime(int i, int t)=0; // returns false if i is out of bounds or time is not inbetween prevoius and next time
	// If t is at a key in the weight curve, this returns the weight at that key.
	// Otherwise, it returns the interpolated weight at time t.
	// If there are no weights, this returns 1.0.
    virtual float GetWeightAtTime(int t)=0;

	// If there is a key at time t on the weight curve, this will reset the weight at that key.
	// If not, this will create a new key at time t with weight w.
	// If this weight curve belongs to a clip, you need to make sure that t is within the 
	// scaled local bounds of the clip. (see note below about the meaning of scaled local)
    virtual int SetWeightAtTime(int t, float w)=0;
};

class IMXtrackgroup: public MaxHeapOperators
{
  public:
    virtual ~IMXtrackgroup() {;}
    virtual int NumTracks()=0;
    virtual IMXtrack *GetTrack(int index)=0;
    virtual const MCHAR *GetName()=0;
    virtual BOOL SetName(const MCHAR *str)=0;
    // the index for the following two filter functions must be one of the follwing track id's defined in track.h:
    // KEY_LARM, KEY_RARM, KEY_LHAND, KEY_RHAND, KEY_LLEG, KEY_RLEG, KEY_LFOOT, KEY_RFOOT, KEY_SPINE, KEY_TAIL, KEY_HEAD,
    // KEY_PELVIS, KEY_VERTICAL, KEY_HORIZONTAL, KEY_TURN, KEY_NECK, KEY_PONY1, KEY_PONY2, KEY_PROP1, KEY_PROP2, KEY_PROP3, 
    virtual BOOL GetFilter(int index)=0;
    virtual BOOL SetFilter(int index, int val)=0;
    virtual void Clear()=0;  // removes all clips from the trackgroup
    virtual BOOL InsertTrack(int index, int tracktype)=0;
    // gets the total time interval of the trackgroup, including all tracks
    virtual BOOL DeleteTrack(int index)=0;
    virtual void GetInterval(Interval& iv)=0;
};

class IMixer: public MaxHeapOperators
{
  public:
		virtual ~IMixer() {;}
    //**************** trackgroups ***********************
    virtual int NumTrackgroups()=0;
    virtual IMXtrackgroup *GetTrackgroup(int index)=0;	
    virtual BOOL InsertTrackgroup(int index)=0;	
    virtual BOOL DeleteTrackgroup(int index)=0;

    //**************** mixer ***********************
    virtual float GetBalancePropagation()=0;  // between 0.0 and 1.0
    virtual BOOL  SetBalancePropagation(float val)=0;
    virtual float GetBalanceLateralRatio()=0; // between 0.0 and 1.0
    virtual BOOL  SetBalanceLateralRatio(float val)=0;
    virtual BOOL  GetBalanceMute()=0;
    virtual BOOL  SetBalanceMute(BOOL val)=0;
    virtual BOOL  IsPerformingMixdown()=0; // if this returns false, the biped is performing a raw mix
    virtual void InvalidateRawMix()=0;
    virtual void EffectRawMix()=0;
    virtual void EffectMixdown()=0;
    // ContinuityRange between 0 and 100, MaxKneeAngle between 0.0 and 180.0
    virtual BOOL Mixdown(BOOL KeyPerFrame, BOOL EnforceIkConstraints, int ContinuityRange, BOOL FilterHyperExtLegs, float MaxKneeAngle)=0;
    virtual void CopyClipSourceToBiped(IMXclip *iCP)=0; 
    virtual void CopyMixdownToBiped()=0; // This will only work if a mixdown exists
    virtual BOOL LoadMixFile(const MCHAR *fname, BOOL redraw)=0;
    virtual BOOL SaveMixFile(const MCHAR *fname)=0;
    // gets the total time interval of the mixer, including all trackgroups and tracks
    virtual void GetInterval(Interval& iv)=0;
    virtual BOOL ExistingMixdown()=0;
	// clear all clips, tracks, and trackgroups - the mixer will be empty
	virtual void ClearMixer()=0;

    // This group of functions actually applies not to the individual mixer,
    // but to the mixer dialog and display.  Many of these just get and set
    // display preferences.
    virtual BOOL   GetSnapFrames()=0;
    virtual void   SetSnapFrames(BOOL onOff)=0;
    virtual BOOL   GetShowTgRangebars()=0;
    virtual void   SetShowTgRangebars(BOOL onOff)=0;
    virtual BOOL   GetShowWgtCurves()=0;
    virtual void   SetShowWgtCurves(BOOL onOff)=0;
    virtual BOOL   GetShowTimeWarps()=0;
    virtual void   SetShowTimeWarps(BOOL onOff)=0;
    virtual BOOL   GetShowClipBounds()=0;
    virtual void   SetShowClipBounds(BOOL onOff)=0;
    virtual BOOL   GetShowGlobal()=0;
    virtual void   SetShowGlobal(BOOL onOff)=0;
    virtual BOOL   GetShowClipNames()=0;
    virtual void   SetShowClipNames(BOOL onOff)=0;
    virtual BOOL   GetShowClipScale()=0;
    virtual void   SetShowClipScale(BOOL onOff)=0;
    virtual BOOL   GetShowTransStart()=0;
    virtual void   SetShowTransStart(BOOL onOff)=0;
    virtual BOOL   GetShowTransEnd()=0;
    virtual void   SetShowTransEnd(BOOL onOff)=0;
    virtual BOOL   GetShowBalance()=0;
    virtual void   SetShowBalance(BOOL onOff)=0;
    virtual BOOL   GetSnapToClips()=0;
    virtual void   SetSnapToClips(BOOL onOff)=0;
    virtual BOOL   GetLockTransitions()=0;
    virtual void   SetLockTransitions(BOOL onOff)=0;
    virtual void   SetAnimationRange()=0;
    virtual void   ZoomExtents()=0;
    virtual void   UpdateDisplay()=0;
    virtual void   AddToDisplay()=0;      // adds this mixer to the mixer dialog
    virtual void   RemoveFromDisplay()=0; // removes this mixer from the mixer dialog
    virtual void   ShowMixer()=0;         // shows the mixer dialog
	virtual void   HideMixer()=0;         // hides the mixer dialog
};


