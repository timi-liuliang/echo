/*==============================================================================
// Copyright (c) 1998-2005 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface to the new Mixer interfaces
// AUTHOR: Michael Zyracki created 2005
//***************************************************************************/

#pragma once
#include "..\maxheap.h"
#include "IMixer.h"
#include "BipedApi.h"
#include "..\ILoadSaveAnimation.h"

//! MAXMOTIONCLIPIMP_CONTROL_CLASS_ID is the Class_ID of the main manager singleton that
//! holds all of the max mixer objects. It resides in a global track in trackview.
#define MAXMOTIONCLIPIMP_CONTROL_CLASS_ID	Class_ID(0x57a52b8c, 0x30935afc)
//! CLIPASSOCIATION_CONTROL_CLASS_ID is the Class_ID of the main class that represents a max mix.
//! It resides with the internal max mixer mananger singleton.
#define CLIPASSOCIATION_CONTROL_CLASS_ID	Class_ID(0x71313526, 0x28696f6a)

//! \brief An interface to the new functionality for max 8 for clips in the mixer. 

//! Contains functionality relating to the new functionality for max clips, including the name of the max map file,
//! and the particular transition information needed to transition max clips. Inherits from IMXclip. The interface
//! is accessed via IMXclip::GetClip8 or IMXclip::GetTransClip8
//! /see IMXtrack8::GetClip8
//! /see IMXtrack8::GetTransClip8
class IMXclip8 : public IMXclip
{
public:
	//! \brief Whether or not a max clip or a biped clip
	//! \return Returns True if it's a max clip, false if it's a biped clip
	virtual bool IsMax() = 0;

	//! \brief The name of the possible map file for a max clip
	//! \return the name of  the map file if one exists, otherwise returns empty string
	virtual const MaxSDK::AssetManagement::AssetUser & MaxMap() =0;

	//! \brief Loads the clip with the given options and animation files. Only works with max clips.
	//! \param[in] loadOption Must be one of the following defined in IMixer.h. RE_SOURCE_CLIP_ONLY, RE_SOURCE_INSTANCES, RE_SOURCE_FILEGROUP. RE_SOURCE_CLIP_ONLY:
	//! the file will be loaded into just this single clip.     RE_SOURCE_INSTANCES: the file will be loaded into this clip and all
	//! instances of this clip. RE_SOURCE_FILEGROUP: the file will be loaded into this clip and all instances and adaptations of this clip.
  	//! \param[in] filename The name of the max animation file(.xaf) to be loaded. It valid .xaf file must
	//! be supplied or the load will fail and the function will return false
	//! \param[in] mapfilename The name of the possible animation map file(.xmm) used for mapping the clip.
	//! This value may be NULL.
	//! \return True if the file was loaded successfully, else returns false
	virtual bool LoadMaxAnimationFile(int loadOption,const MCHAR *filename, const MCHAR *mapfilename=NULL) =0;

	//! \name Transition Clip Information
	//@{

	//! \brief Get the nodes that make up the velocity blended nodes for a transition.  Only works with max clips.
	
	//! \param[out] nodeTab The nodes whose positions and orientations are accumulated and blended via velocity
	//! for this clips transition
	virtual void GetVelocityBlends(INodeTab &nodeTab) =0;
	
	//! \brief Set the nodes that make up the velocity blended nodes. Only works with max clips.

	//! \param[in] nodeTab The nodes whose positions and orientations are accumulated and bleneded via their
	//! velocities in this clip
	virtual void SetVelocityBlends(const INodeTab &nodeTab) =0;
	//@}

	//! \name SubBlendNodes
	//@{

	//! \brief Get the nodes that make up the the sub blended nodes. Only works with max clips.

	//! \param[out] nodeTab Nodes who will get their positions and orientations blended in the coordinate space of their corresponding
	//! subBlendNode parent thats found by calling GetSubBlendNodesParents.
	virtual void GetSubBlendNodes(INodeTab &nodeTab) =0;

	//! \brief Set the nodes that make up the sub blended nodes. Only works with max clips.

	//! \param[in] nodeTab Nodes who will get their positions and orientations blended in the coordinate space of their corresponding
	//! subBlendNode parent.  You should call this functions first before calling SetSubBlendNodesParents.
	virtual void SetSubBlendNodes(const INodeTab &nodeTab) =0;

	//! \brief Get the nodes that make up the sub blended nodes parents. Only works with max clips.

	/*! \param[out] nodeTab An array of 'parent' nodes whose coordinate space the sub blend nodes will blend in. */
	virtual void GetSubBlendNodesParents(INodeTab &nodeTab) =0;

	//! \brief Set the nodes that make up the sub blended parent nodes. Only works with max clips.

	//! \param[in] nodeTab The nodes whose coordinate space the sub blend nodes will blend in . Should be called after SetSubBlendNodeParents,
	//! and there's a 1-1 correspondance between the the two arrays.
	virtual void SetSubBlendNodesParents(const INodeTab &nodeTab) =0;
	//@}

	//! \brief Set whether or not velocity blended nodes yaw,pitch and roll angles will get accumulated or not. Only works with max clips.
	//! \param[in] yaw If true the yaw orientation (angle about local z)will get velocity blended, otherwise it will get blended normally.
	//! \param[in] pitch If true the pitch orientation(angle about local x) will get velocity blended, otherwise it will get blended normally.
	//! \param[in] roll If true the roll orientation(angle about local y) will get velocity blended, otherwise it will get blended normally.
	virtual void SetTransitionYawPitchRoll(const bool yaw, const bool pitch,const bool roll) =0;

	//! \brief Get whether or not velocity blended nodes yaw,pitch and roll angles will get accumulated or not. Only works with max clips.
	//! \param[out] yaw If true the yaw orientation (angle about local z)gets velocity blended, otherwise it will get blended normally.
	//! \param[out] pitch If true the pitch orientation(angle about local x) gets velocity blended, otherwise it will get blended normally.
	//! \param[out] roll If true the roll orientation(angle about local y) gets velocity blended, otherwise it will get blended normally.
	virtual void GetTransitionYawPitchRoll(bool &yaw, bool &pitch, bool &roll) =0;

	//! \brief Set the transition yaw,pitch and roll angles.
	//! \param[in] yaw The additional yaw angle the clip will transition around
	//! \param[in] pitch The additional pitch angle the clip with transition around
	//! \param[in] roll The additional roll angel the clip will transition around
	virtual void SetTransitionYawPitchRollAngle(const float yaw, const float pitch,const float roll) =0;

	//!\brief Get the transition yaw,pitch and roll angles.
	//! \param[out] yaw The additional yaw angle of the current transition.
	//! \param[out] pitch The additional pitch angle of the curren transition.
	//! \param[out] roll The additional roll angel of the current transition
	virtual void GetTransitionYawPitchRollAngle(float &yaw, float &pitch, float &roll) =0;

	//!\brief Set whether or not velocity blended nodes x,y, and z positions will get accumulated or not. Only works with max clips.
	//! \param[in] x If true the x position, will get velocity blended, otherwise it will get blended normally.
	//! \param[in] y If true the y position, will get velocity blended, otherwise it will get blended normally.
	//! \param[in] z If true the z position, will get velocity blended, otherwise it will get blended normally.
	virtual void SetTransitionXYZ(const bool x, const bool y,const bool z) =0;

	//! \brief Get whether or not velocity blended nodes x,y and z positions will get accumulated
	//! \param[out] x If true the x position gets velocity blended, otherwise it will get blended normally.
	//! \param[out] y If true the y position gets velocity blended, otherwise it will get blended normally.
	//! \param[out] z If true the z position gets velocity blended, otherwise it will get blended normally.
	virtual void GetTransitionXYZ(bool &x, bool &y, bool &z) =0;



};


//! \brief An interface to the new functionality for max 8 for tracks in the mixer.

//! Contains functionality relating to the new functionality for max tracks, mainly the ability to get or append new
//! IMXclip8 clips.  Inherits from IMXtrack. The interface is accessed via IMXtrackgroup8::GetTrack8.
//! /see IMXtrackgroup8::GetTrack8
class IMXtrack8 : public IMXtrack
{
public:

	//! \brief Returns the IMXclip8 interface for the specified clip
	//! \param[in] index Which clip you want to get, >=0.
	//! \param[in] row Which row the clip you want to get is in, value may be BOT_ROW or TOP_ROW
	//! defined in IMixer.h  For layer tracks, only BOT_ROW is a valid input.
	//! \return The new clip interface for this clip. May return NULL if an invalid
	//! parameter,either improper row or index, is passed in.
	
	virtual IMXclip8 *GetClip8(int index, int row)=0;
	
	//! \brief Returns the IMXclip8 interface for the specified clip.
	//! \param[in] index The index of the sorted clips in the transition track. Takes into account both
	//! rows when indexing the clips.
	//! \return The new clip interface for this clip. May return NULL if an invalid
	//! parameter is passed in.
	virtual IMXclip8 *GetTransClip8(int index)=0;

	//! \brief Whether or not a max track or a biped track.
	//! \return Returns true if it's a max track, false if it's a biped track.
	virtual bool IsMax() = 0;

	//! \brief Loads the specified max animation file as a clip onto the track. Only works for max tracks.
	//! \param[in] fname The filename of the .xaf file to be loaded.
	//! \param[in] mapFile The filename of the .xmm map file to be loaded. Defaults to NULL,
	//! \param[in] interval The number of frames from the end of the last clipin the track that the new clip will be appended
	//! in layer tracks. The interval must be >=0. Not that for transistion tracks, this parameter is ingnored and the start
	//! of the new clip will be at theinpoint of the last clip in the track, and it will be on the opposite row of the last clip.
	//! Also the parameter is ignored if there are no clips in the track, the clip will automatically start at frame 0.
	//! \return false if the files can't be loaded or the mapping isn't correct. If the clip is created properly, returns true.
	virtual bool AppendMaxClip(const MCHAR *fname, const MCHAR *mapFile = NULL, int interval = 5)=0;

};


//! \brief An interface to the new functionality for max 8 for trackgroups in the mixer. In particularly,
//! new information for max trackgroups.

//! Contains functionality relating to the new functionality for max trackgrouops, mainly the new filter for max mixes.
//! Inherits from IMXtrackgroup. The interface is accessed via IMXclip::GetTrackgroup8
//! /see IMixer8::GetTrackgroup8
class IMXtrackgroup8 : public IMXtrackgroup
{
public:

	//! \brief Whether or not a max trackgroup or a biped trackgroup.
	//! \return Returns true if it's a max trackgroup, false if it's a biped trackgroup.
	virtual bool IsMax() = 0;

	//! \brief Returns the IMXtrack8 interface for the specified clip.
	//! \param[in] index The index of the track.
	//! \return The new track interface for this clip. May return NULL if an invalid
	//! index is passed in.
    virtual IMXtrack8 *GetTrack8(int index)=0;


	//! \brief Get the nodes that are filtered in the current max mix. Only works with max trackgroups.
	//! \param[out] nodeTab The nodes which are filtered in the max mix.
	virtual void GetMaxFilter(INodeTab &nodeTab) =0;
	
	//! \brief Set the filtered nodes for this trackgroup. Only works with max trackgroups.
	//! \param[in] nodeTab The nodes that you want to be filtered in this trackgroup.
	virtual void SetMaxFilter(const INodeTab &nodeTab) =0;

};

//! \brief An interface to the new functionality for max 8 for the mixer. In particularly,
//! new information for max mixers.

//! Contains functionality relating to the new functionality for max mixers, including setting and getting the name and
//! the nodes in a max mixer.   The interface is accessed via TheMaxMixerManager.GetNthMaxMixer or
//! TheMaxMixerManger.GetMaxMixer
//! /see MaxMixerManager::GetNthMaxMixer
//! /see MaxMixerManager::GetMaxMixer
class IMixer8 :public IMixer
{

public:
	//! \brief Whether or not a max mixer or a biped mixer.
	//! \return Returns true if it's a max mixer, false if it's a biped mixer.
	virtual bool IsMax() = 0;

	//! \brief Returns the IMXtrackgroup8 interface for the specified clip.
	//! \param[in] index The index of the trackgroup.
	//! \return The new trackgroup interface for this clip. May return NULL if an invalid
	//! index is passed in.
    virtual IMXtrackgroup8 *GetTrackgroup8(int index)=0;	

	//! \brief Set the nodes for the max mixer. This function does nothing for biped mixers.
	//! \param[in,out] nodeTab The nodes that the mixer will now contain. Note that a node may only be in one mix at a
	//! time. The nodes that the new max mix now contains will be returned in this tab. 
	//! \param[in] collapse If TRUE than any node that is no longer part of the max mixer will collapse the animation
	//! thats in the mixer onto it's mixed tracks. If FALSE than the original animation that was on these nodes when the
	//! node was added to the mixer will now revert back onto the node.
	virtual void SetNodes(INodeTab &nodeTab,BOOL collapse) =0;

	//! \brief Get the nodes for this max mixer. This function does nothing for biped mixers.
	//! \param[out] nodeTab The nodes that make up this max mixer. 
	virtual void GetNodes(INodeTab &nodeTab)=0;

	//! \brief Get the name of the mixer.
	//! \return The name of the mixer.
	virtual const MCHAR *GetName() = 0;
	
	//! \brief Set the name of the mixer
	//! \param[in] name The name of the mix. Note that you can only set the name
	//! name of max mixes and not biped mixes since their name cames from the name of the biped root node. 
	//! \return Returns truue if the name was changed correctly, returns false if the name wasn't
	//! changed either because the mix is a biped mix or the name of the mix already exists in the mixer.
	virtual bool SetName(const MCHAR *name) =0;

};

//! \brief A class that allows a user to access the new max mixers in max 8

//! This class allows the user to create, access, and remove max mixes from the mixer.
//! The class exists in the scene as a single global variable, TheMaxMixerManager.
class MaxMixerManager: public MaxHeapOperators
{
public:
	//! \brief Destructor
	virtual ~MaxMixerManager() {;}

	//!\brief Returns the Number of Max Mixers in the mixer.
	//!\return The number of Max Mixers in the mixer.
	BIPExport int NumMaxMixers();
	
	//! \brief Get's the Nth max mixer
	//! \param[in] index The index of the IMixer8 interface that's returned. If the index is less than zero
	//! or greater than the number of max mixers, then NULL will be returned.
	//! \return The mixer interface for this max mixer.
	BIPExport IMixer8 *GetNthMaxMixer(int index);
	
	//! \brief Get's the max mixer that has this node in it.
	//! \param[in] node Get the max mixer that this node in it. If this node isn't in any mixers, then NULL will be returned.
	//! \return The mixer interface for this max mixer.
	BIPExport IMixer8 *GetMaxMixer(INode *node);

	//! \brief Remove the nth max mixer from the mixer, and subsequently either collapse the mixed animation onto the
	//! the old max mixer nodes, or replace the animation with the original animation that was there.
	//! \param[in] index The index of the max mixer to be removed. If the index is less than zero
	//! or greater than the number of max mixers, then nothing will happen.
	//! \param[in] collapse If TRUE the mixer will collapse and replace the mixer slave controllers with
	//! the values set in the mixer, otherwise it will replace the slaves with the original controllers
	//! and their values, when the nodes were added to the mixer.
	//! \param[in] keyperframe If TRUE the mixer will collapse with a keyperframe, if FALSE, the collapse
	//! will try to collapse individual keyframes from the clips instead.
	BIPExport void RemoveNthMaxMixer(int index,BOOL collapse,BOOL keyperframe);
	
	//! \brief Add a new max mixer to the mixer using a set of nodes
	//! \param[in] nodes The nodes to be added to the mixer.
	//! \param[in] nameOfMix The name of the max mix. Note that if the name already exists
	//! the function will return false and the new max mix won't be created.
	//! \return Will return false if the nameOfMix parameter specifies a name of a mix
	//! that already exists, and will return true if the max mixer is successfully created.
	BIPExport bool AddMaxMixer(INodeTab &nodes, const MCHAR *nameOfMix);
	
	//! \brief Add a new max mixer to the mixer via list of nodes and specific tracks you want to add
	//! \param[in,out] nodeAnims A tab of the NodeAndAnims structures to be added to the mixer. This provides more flexibility
	//! in specifying which tracks you really want to be mixed or not.  NodeAndAnims is defined in ILoadSaveAnimation.h  Note
	//! that if a node is found in another mix we will delete it, and the resulting nodeAnims tab after the function call
	//! will contain those nodeAnims currently still existing in the maxObject.
	//! \see NodeAndAnims
	//! \param[in] nameOfMix The name of the max mix. Note that if the name already exists
	//! the function will return false and the new max mix won't be created.
	//! \return Will return false if the nameOfMix parameter specifies a name of a mix
	//! that already exists, otherwise if the mix is created successfully it will return true
	BIPExport bool AddMaxMixer(Tab<NodeAndAnims> &nodeAnims,const MCHAR *nameOfMix);

};

//! \brief Global object that contains the new MaxMixerManager object 
extern BIPExport MaxMixerManager TheMaxMixerManager;





