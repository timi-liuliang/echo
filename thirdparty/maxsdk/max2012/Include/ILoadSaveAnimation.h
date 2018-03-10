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
// DESCRIPTION: Interface to Load Save Animation Interfaces
// AUTHOR: Michael Zyracki created 2005
//***************************************************************************/

#pragma once
#include "Maxapi.h"
#include "maxheap.h"
#include "ifnpub.h"
#include "iparamb2.h"
#include "XMLAnimTreeEntry.h"


//! \brief Class used to input retargeting info when creating a map file.

//! Contains info on how a particular mapped node will get retargeted.  You specify a node that's being retargeted, and then parameters that
//! specify how the node will get retargeted, ie the amount to scale the position and the coordinate space where the node will get scaled.
//! This class is used in ILoadSaveAnimation::CreateMapFile and ILoadSaveAnimation::CreateMapFileNodeAnims
//! \see ILoadSaveAnimation::CreateMapFile
//! \see ILoadSaveAnimation::CreateMapFileNodeAnims
class LoadSaveRetargetData: public MaxHeapOperators
{
	public:

	//! \brief Constructor
	LoadSaveRetargetData():absolute(TRUE){};

	// !\brief Deconstructor
	~LoadSaveRetargetData(){}

	// !\brief The name of the node in the scene that's being mapped.  This must be set.
	MSTR currentNodeName;

	//! \brief The name of the incoming node from the animation file that the current node is mapped to. Note that the map file
	//! that uses this information still needs to explicitly map the tracks from the incoming tracks to the current tracks. This
	//! information doesn't override that information, just confirms it.
	MSTR incomingNodeName;
	
	//! \brief  Whether or not the scale attribute will be the absolute scale value or a relative scale value that is then multiplied
	//! by a scale that's calculated from the ratio of the length of specified current linked nodes versus the length of specified incoming linked nodes.
	BOOL absolute;
	
	//! \brief The scale attribute that's used to scale the retargeted position. If absolute is false then it's a relative value that's multiplied by
	//! the ratio of current versus incoming chain lengths.
	Point3 scale; 


	//! \brief The name of the base node whose coordinate space you will scale in.  If the current base node is the same as the current node that's being
	//! being retargeted, then the node will be retargeted relative to it's first position.
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR currentBase;

	//! \brief The name of the base node whose coordinate space you will scale the incoming node in.  
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR incomingBase;

	//! \brief The name of the base of the current chain node that will be used to calculate the scale for retargeting when the absolute
	//! value is false. The scale is then the difference in length from the current end chain to it's base versus the incoming end chain
	//! to it's base.
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR currentBaseChain;

	//! \brief The name of the end chain of the current chain node that the scale will be calculated from.  The length is calculated
	//! by traversing the distances from the end chain to it's parents, down to the base chain node.
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR currentEndChain;


	//! \brief The name of the base of the incoming chain node that the scale will be calculated from.  
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR incomingBaseChain;

	//! \brief The name of the end of the incoming chain node that the scale will be calculated from.  The length is calculated
	//! by traversing the distances from the end chain to it's parents, down to the base chain node.
	//! Note that if this value isn't set, then the world root node will be this node.
	MSTR incomingEndChain;


	//! \brief If TRUE the current FK node will retarget internally as a IK chain to a specified node. That way when retargeting FK chains we can
	//! correctly make sure that the end nodes(like feet) will stay planted on the ground relative to the center of mass.
	BOOL FKRetargetingEnabled; 
	
	//! \brief The name of the node where the base of the internal IK chain will end.  So if you were retargeting a foot node, you would usually specify
	//! this node to be the hip, so that when the foot retargets as the end of an IK chain, the ankle, and knee joints will get recalculated.
	//! Remember you only need to do this for FK nodes when you want them to be constrained relative to another node, usually the\
	//! center of mass.  Note that if this node isn't set, then this sort of retargeting won't occur, even if the FKRetargetingEnabled flag is TRUE.
	MSTR FKChainBase;

};


//! \brief Interface ID for the ILoadSaveAnimation Interface and also for the ILoadSaveAnimation10 Interface
//! \see ILoadSaveAnimation
//! \see ILoadSaveAnimation10
#define ILOADSAVEANIMATION_INTERFACE Interface_ID(0x7e1b6067, 0x6f470508)

//! Helper method for getting the ILoadSaveAnimation10 interface
#define GetILoadSaveAnimation()	static_cast<ILoadSaveAnimation*>(GetCOREInterface(ILOADSAVEANIMATION_INTERFACE))


//! \brief The interface to the functionality for loading saving XML animation files.

//! This class contains key functionality for saving and creating both animation and map files
//! and for loading in animation.
//! You get this interface by calling GetILoadSaveAnimation()
class ILoadSaveAnimation : public FPStaticInterface {

public:

	//! \name Enums
	//@{

	//! \brief Enum for the type of node mapping to due when creating a map file using CreateMapFile
	
	//! \see ILoadSaveAnimation::CreateMapFile
	//! eMatchExactName means that the nodes names need to match up exactly to map, eMatchClosestName
	//! specifies that the closest names will map, eMatchHierarchy specifies that the nodes will try
	//! to match by hierarchy.
	enum NodeMapType{ eMatchExactName=0, eMatchClosestName=1,eMatchHierarchy=2};

	//! \brief Enum for different flags to pass in when calling ILoadSaveAnimation::SaveAnimation and
	//! ILoadSaveAnimation10::SetUpAnimsForSaveExt

	//! \brief If eAnimatedTracks is set, then a track needs to be animated. If eIncludeConstraints is set, and if
	//! eAnimatedTracks is set, constraints will also be considered to be animated. If eKeyable is set, then
	//! keyable tracks will be saved if eSaveSeg is set, then a segment of time will be saved. if eSegPerFrame
	//! is set then a key per frame will be saved over a saved segment.
	enum SaveFlags {eAnimatedTracks = 0x1, eIncludeConstraints = 0x2, eKeyableTracks = 0x4,eSaveSeg = 0x8, 
		eSegPerFrame = 0x10};

	//! \brief Enum for different flags to pass in when calling ILoadSaveAnimation::LoadAnimation and 
	//! ILoadSaveAnimation10::SetUpAnimsForLoadExt
	
	//! If eRelative is set, then nodes who are children of the scene root will keep their original
	//! position and orientation and the animation will be loaded relatively to that, otherwise the
	//! animation will be loaded in absolutely. If eInsert is set, then the animation will be inserted
	//! at the specified time and the existing animation will be moved over, otherwise the animation will
	//! replaced the existing animation at that time.  If eStripLayers is set, then the active layer name
	//! will be stripped out from the current node controllers that have layers.  If eIncludePB2s is set then
	//! we will include ParamBlock2 tracks when loading into the animation. Note that the eIncludePB2s flags is currently 
	//! only used by ILoadSaveAnimation10::SetUpAnimsForLoadExt
	enum LoadFlags {eRelative = 0x1,eInsert = 0x2, eStripLayers = 0x4, eIncludePB2s = 0x8};

	//! \brief Enum for different flags to pass in when calling ILoadSaveAnimation::CreateMapFile and
	//! ILoadSaveAnimation10::SetUpAnimsForMapExt

	//! If eMapControllerExactName is set then the controllers will try to be matched based upon their exact names,
	//! otherwise, they will try to match off of their subanim order. If eMapControllerType is set then the controller need to
	//! be the same of the same class, otherwise they won't map.   If eStripLayersInMap is set, then the active layer name will be removed
	//! from the current node controllers that have layers.  
	enum MapFlags {eMapControllerExactName = 0x1,eMapControllerType = 0x2,eStripLayersInMap = 0x4};
	//@}


	//! \name Dialog Functions
	//@{

	//! \brief Brings up the Save Animation dialog. 
    /*! \return The name of the file if a file gets saved, otherwise returns NULL !*/
	virtual const MCHAR* SaveAnimationDlg() const =0;

	//! \brief Brings up the Load Animation dialog.
    /*! \return  The name of the file if a file gets loaded, otherwise returns NULL !*/
	virtual const MCHAR* LoadAnimationDlg() const =0;

	//! \brief Brings up the Load Animation dialog and returns the fileName and the mapFileName in the passed in parameters.
	/*! \param[out] fileName The name of the animation file returned. Note that the character array passed in
	//! should have a size of MAX_PATH.
	//! \param[out] mapFileName The name of the map file returned.  Note that the character array passed in
	//! should have a size of MAX_PATH.
    //! \return TRUE if a file name is selected in the dialog, otherwise returns FALSE.
	!*/
	virtual BOOL GetLoadAndMapFileFromDlg(MCHAR fileName[MAX_PATH],MCHAR mapFileName[MAX_PATH]) const =0;

	//! \brief Brings up the Load Animation dialog and returns a list of possible multiple file names in the passed in parameters.
	
	//! param[out] names The list of animation files.
	//! param[out] mapFileName The name of the map file. Note that the character array passed in should have a size of MAX_PATH.
	//! \return TRUE if one or more file names are selected, false otherwise
	virtual BOOL GetMultLoadAndMapFileFromDlg(Tab<MSTR*> &names, MCHAR *mapName) const = 0;
	//@}


	//! \name Save Functions
	//@{

	//! \brief Save the animation from the current nodes into the specified file. 
	
	//! This function will save out an animation file given the passed in nodes and user attributes, based
	//! upon the saveFlags and possible segment interval.
	/*! \param[in] filename The file to save the animation to. 
	//! \param[in] nodes The nodes whose animation you would like to save.
	//! \param[in] userAttributes A Tab of strings that specify the names of the user attributes to be
	//! saved out.  The values of these attributes are found in the next parameter. Thus the userAttributes Tab
	//! and the userValues Tab need to be the same size or the function will return FALSE.
	//! \param[in] userValues A Tab of strings that specify the names of the user values to be saved out. Each
	//! user value corresponds to a user attribute that's passed in the previous parameter.
	//! \param[in] saveFlag Determines which tracks under the specified nodes will be saved.  The possible values
	//! of the saveFlag are found in the SaveFlags enum. The possible vales are eAnimatedTracks, if this value is set, 
	//! then only animated tracks only will be saved, otherwise non-animated tracks will also be saved.
	//! eIncludeConstraints, if set and eAnimatedTracks is set, then constraints will also be saved out,
	//! otherwise if eAnimatedTracks is set and this value isn't set, they won't be saved out.  If eAnimatedTracks isn't
    //! set this parameter is unused since constraints will be automatically saved out. eKeyableTracks, if set keyable 
	//! tracks only will be saved, otherwise non-keyable tracks will also be saved. eSaveSegment, if set a segment will
	//! be saved specified by the segInterval and eSegKeyPerFrame flag. eSegKeyPerFrame, which if set
	//! will save out a key per frame when saving out a segment, otherwise only the keys will be saved out.
	//! \param[in] segInterval The interval over which to save the segment.
	//! \return TRUE if a file gets saved, otherwise FALSE
	!*/
	virtual BOOL SaveAnimation(const MCHAR *filename, Tab<INode *> &nodes,Tab<MSTR*> &userAttributes,Tab<MSTR*> &userValues,
		DWORD saveFlag, 
		Interval &segInterval) const = 0;
	
	//! \brief Save the animation from the current nodeAnim tracks into the specified file. 
	
	//! This function saves out the animation file given the passed in nodeAnim array and user attributes, based
	//! upon the saveFlags and possible segment interval.
	/*! \param[in] filename The file to save the animation to. 
	//! \param[in] nodeAnims The nodeAnim list whose animation you would like to save. Should have been created via
	//! SetUpAnimsForSave.
	//! \see ILoadSaveAnimation::SetUpAnimsForSave
	//! \param[in] userAttributes A Tab of strings that specify the names of the user attributes to be
	//! saved out.  The values of these attributes are found in the next parameter. Thus the userAttributes Tab
	//! and the userValues Tab need to be the same size or the function will return FALSE.
	//! \param[in] userValues A Tab of strings that specify the names of the user values to be saved out. Each
	//! user value corresponds to a user attribute that's passed in the previous parameter.
	/*! \param[in] saveFlag Determines which tracks under the specified nodes will be saved.  The possible values
	of the saveFlag are found in the SaveFlags enum. The possible vales are: eAnimatedTracks, if this value is set, 
	then only animated tracks only will be saved, otherwise non-animated tracks will also be saved.
	eIncludeConstraints, if set and eAnimatedTracks is set, then constraints will also be saved out,
	otherwise if eAnimatedTracks is set and this value isn't set, they won't be saved out.  If eAnimatedTracks isn't
    set this parameter is unused since constraints will be automatically saved out. eKeyableTracks, if set keyable 
	tracks only will be saved, otherwise non-keyable tracks will also be saved. eSaveSegment, if set a segment will
	be saved specified by the segInterval and eSegKeyPerFrame flag. eSegKeyPerFrame, which if set
	will save out a key per frame when saving out a segment, otherwise only the keys will be saved out.
	//! \param[in] segInterval The interval over which to save the segment.
	//! \return TRUE if a file gets saved, otherwise FALSE
	!*/
	virtual BOOL SaveAnimationNodeAnims(const MCHAR *filename, Tab<NodeAndAnims > &nodeAnims,Tab<MSTR*> &userAttributes,Tab<MSTR*> &userValues,
		DWORD saveFlag, Interval &segInterval) const = 0;
	//@}

	//! \name Load Functions
	//@{

	//! \brief Load the animation from the current file onto the specified nodes using any mapping that's specified.
	
	//! This function loads in the specified file onto the passed in nodes based upon the the passed in flags,
	//! insertion time and possible mapFileName.
	/*! \param[in] filename The file to load the animation from. 
	//! \param[in] nodes The nodes where the animation will be loaded onto.
	//! \param[in] loadFlag Determines which tracks under the specified nodes will be loaded onto.  The possible values
	//! of the loadFlag are found in the LoadFlags enum. The possible vales are: eRelative, if this value is set,	
    //! the animation will be loaded at the current position and orientation of a node, if it's
	//! parent is the root, if not set the animation will be loaded in absolutely. eInsert, if this value is set,
	//! the existing animation will be moved to the left when the animation is loaded, otherwise is not set the
	//! animation will overwrite the existing animation. eStripLayers, if this value is set, then any layer controller name
	//! will be removed from the current controller names, otherwise it won't.
	//! \param[in] insertTime The time where the animation is loaded.
	//! \param[in] useMapFile If TRUE the mapFileName file name parameter will be used to specify what map file to use. 
	//! If FALSE then default mapping is used.
	//! \param[in] mapFileName The file name of the map file to use if the useMapFile parameter is TRUE.
	//! \return TRUE if a file gets loaded, otherwise FALSE
	!*/
	virtual BOOL LoadAnimation(const MCHAR *filename,Tab<INode *> &nodes,DWORD loadFlag, TimeValue insertTime,
		BOOL useMapFile, MCHAR *mapFileName) const = 0;

	//! \brief Load the animation from the current file onto the specified nodeAnim tracks using any mapping that's specified.
	
	//! This function loads in the specified file onto the passed in nodeAndAnim tracks  based upon the the passed in flags,
	//! insertion time and possible mapFileName.
	/*! \param[in] filename The file to load the animation from. 
	//! \param[in] nodeAnims The nodeAnim list whose animation you would like to save. Should have been created via
	//! SetUpAnimsForLoad. 	\see ILoadSaveAnimation::SetUpAnimsForLoad
	//! \param[in] loadFlag Determines which tracks under the specified nodes will be loaded onto.  The possible values
	//! of the loadFlag are found in the LoadFlags enum. The possible vales are: eRelative, if this value is set,	
    //! the animation will be loaded at the current position and orientation of a node, if it's
	//! parent is the root, if not set the animation will be loaded in absolutely. eInsert, if this value is set,
	//! the existing animation will be moved to the left when the animation is loaded, otherwise is not set the
	//! animation will overwrite the existing animation. 
	//! \param[in] insertTime The time where the animation is loaded.
	//! \param[in] useMapFile If TRUE the mapFileName file name parameter will be used to specify what map file to use. 
	//! If FALSE then default mapping is used.
	//! \param[in] mapFileName The file name of the map file to use if the useMapFile parameter is TRUE.
	//! \return TRUE if a file gets loaded, otherwise FALSE
	!*/
	virtual BOOL LoadAnimationNodeAnims(const MCHAR *filename,Tab<NodeAndAnims> &nodeAnims,DWORD loadFlag, TimeValue insertTime,
		BOOL useMapFile, MCHAR *mapFileName) const = 0;
	//@}

	//! \name Map Functions
	//@{

	//! \brief Create a map file based upon nodes to map to, the animation file that will be loaded, and the specific mapping protocols.
	
	//! \param[in] filename The file to save the map file to. 
	//! \param[in] currentNodes The nodes which will be mapped to the specified incoming Animation File.
	//! \param[in] incomingAnimationFile The file from which to map from. The controllers in this file will map to the 
	//! controllers of the current nodes.
	//! \param[in] userAttributes A Tab of strings that specify the names of the user attributes to be
	//! saved out.  The values of these attributes are found in the next parameter. Thus the userAttributes Tab
	//! and the userValues Tab need to be the same size or the function will return FALSE.
	//! \param[in] userValues A Tab of strings that specify the names of the user values to be saved out. Each
	//! user value corresponds to a user attribute that's passed in the previous parameter.
	//! \param[in] retargetData A Tab of LoadSaveRetargetData values that specify how specific nodes that get mapped should be retargeted.
	//! \param[in] nodeMapType The type of node mapping to perform. If the value is eMatchExactName, the nodes will map
	//! if the nodes have the same exact name. If the value is eMatchClosestName, the nodes will match based off of 
	//! which nodes have the closest names. If the value is eMatchHierarchy, the nodes will try to match by matching their
	//! hierarchies.
	//! \param[in] mapFlag Determines how the controller mapping will occur once the nodes are mapped.  The possible
	//! values of the mapFlag are found in the MapFlags enum. The possible vales are: eMapControllerExactName, 
	//! if set the controllers of the mapped nodes will try to map to each other by  matching their names exactly,
	//! if not set the controllers will map by their subanim ordering. eMapControllerType, if set, the controllers 
	//! will check to see if they have the same exact type, that is the same ClassID and SuperClassID, otherwise they won't map, if not set
	//! then the controllers only need to be have the same SuperClassID. eStripLayersinMap, if set, will strip the layer controller name
	//! from any active layers on the current nodes.
	//! \return TRUE if a map file gets created, otherwise FALSE
	virtual BOOL CreateMapFile(const MCHAR *filename, Tab<INode *> &currentNodes,const MCHAR *incomingAnimationFile,
		Tab<MSTR*> &userAttributes,Tab<MSTR*> &userValues,Tab<LoadSaveRetargetData> &retargetData, int  nodeMapType, 
		DWORD mapFlag) const =0;

	//! \brief Create a map file based upon nodeAnim tracks to map to, the animation file that will be loaded, and the specific mapping protocols.
	
	//! \param[in] filename The file to save the map file to. 
	//! \param[in] nodeAnims The nodeAnim list whose animation you would like to map. Should have been created via
	//! SetUpAnimsForMap.
	//! \see ILoadSaveAnimation::SetUpAnimsForMap
	//! \param[in] incomingAnimationFile The file from which to map from. The controllers in this file will map to the 
	//! controllers of the current nodes.
	//! \param[in] userAttributes A Tab of strings that specify the names of the user attributes to be
	//! saved out.  The values of these attributes are found in the next parameter. Thus the userAttributes Tab
	//! and the userValues Tab need to be the same size or the function will return FALSE.
	//! \param[in] userValues A Tab of strings that specify the names of the user values to be saved out. Each
	//! user value corresponds to a user attribute that's passed in the previous parameter.
	//! \param[in] retargetData A Tab of LoadSaveRetargetData values that specify how specific nodes that get mapped should be retargetted.
	//! \param[in] nodeMapType The type of node mapping to perform. If the value is eMatchExactName, the nodes will map
	//! if the nodes have the same exact name. If the value is eMatchClosestName, the nodes will match based off of 
	//! which nodes have the closest names. If the value is eMatchHierarchy, the nodes will try to match by matching their
	//! hierarchies.
	//! \param[in] mapFlag Determines how the controller mapping will occur once the nodes are mapped.  The possible
	//! values of the mapFlag are found in the MapFlags enum. The possible vales are: eMapControllerExactName, 
	//! if set the controllers of the mapped nodes will try to map to each other by  matching their names exactly,
	//! if not set the controllers will map by their subanim ordering. eMapControllerType, if set, the controllers 
	//! will check to see if they have the same exact type,(ClassID and SuperClassID), otherwise they won't map, if not set,
	//! then the controllers only need to be have the same SuperClassID.
	//! \return TRUE if a map file gets created, otherwise FALSE
	virtual BOOL CreateMapFileNodeAnims(const MCHAR *filename, Tab<NodeAndAnims> &nodeAnims,const MCHAR *incomingAnimationFile,
		Tab<MSTR*> &userAttributes,Tab<MSTR*> &userValues,Tab<LoadSaveRetargetData> &retargetData, int  nodeMapType, 
		DWORD mapFlag) const =0;
	//@}

	//! \brief Get the user attributes that are present out of the corresponding animation or map file

	//! \param[in] filename The name of the animation or map file that you are getting user attributes from.
	//! \param[out] userAttributes A Tab of MSTR strings of the user defined attributes found in the file. There is a 
	//! one to one correspondence between the number of attributes and number of values in the file. The callee of this
	//! function is responsible for deleting the valid MSTR * pointers.
	//! \param[out] userValues A Tab of MSTR strings of the user defined values found in the file. There is one string value
	//! for each attribute.  The caller of this function is responsible for deleting the valid MSTR * pointers
	//! \return TRUE if operation was successful, FALSE if otherwise.
	virtual BOOL GetUserAttributes(const MCHAR *filename, Tab<MSTR*> &userAttributes, Tab<MSTR*> &userValues) const =0;


	//! \name File and Directory Functions
	//@{
	
	//! \brief Get the extension used for the animation files.
	//! \return The animation file extension (xaf).
	virtual const MCHAR * GetAnimFileExtension() const = 0;

	//! \brief Get the extension used for the animation map files.
	//! \return The animation map file extension (xmm).
	virtual const MCHAR * GetAnimMapFileExtension() const = 0;

	//! \brief Get the current directory for finding animation files.
	//! \return The animation file directory
	virtual const MCHAR * GetAnimFileDirectory() const = 0;

	//! \brief Get all of the anim file directories in the load/save UI.
	
	//! \param[out] animFileDirectories A Tab of MSTR strings containing all of recently used directories. The caller of this
	//! function is responsible for deleting the valid MSTR * pointers.
	virtual void GetAnimFileDirectories(Tab<MSTR*> &animFileDirectories) const = 0;

	//! \brief Get all of the recently used map files in the load/save UI.

	//! \param[out] mapFiles  A Tab of MSTR strings containing all of recently used map files. The caller of this
	//! function is responsible for deleting the valid MSTR * pointers.
	virtual void GetMapFiles(Tab<MSTR*> &mapFiles) const = 0;

	//! \brief Get the current map file.
	//! \return The name of the current animation map file being used in the load/save UI.
	virtual const MCHAR * GetAnimMapFile() const = 0;

	//! \brief Set the current directory for finding animation files.
	//! \param[in] directory  The name of the anim file directory. 
	virtual void SetAnimFileDirectory(const MCHAR *directory)=0;
	
	//! \brief Set the latest map file found 
	//! \param[in] file The name of the map file.
	virtual void SetAnimMapFile(const MCHAR *filename)=0;
	//@}


	//! \name Getting Animation Tracks
	//@{

	//! \brief  Sets up the anim list to be used for saving out an xml animation file.
	
	//! \param[in] nodeTab Nodes you want to save out
	//! \param[in] animatedTracks If TRUE will only included animated tracks in the list,
	//! \param[in] includeConstraints If animatedTracks is TRUE and this is TRUE, constraints will be considered
	//! to be animated and thus also in the list.
	//! \param[in] keyable If TRUE will only include keyable tracks in the list.
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks based upon the flags passed into the function.
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::SaveAnimationNodeAnims
	virtual void SetUpAnimsForSave(Tab<INode *> &nodeTab,BOOL animatedTracks, BOOL includeConstraints, BOOL keyable,Tab<NodeAndAnims> &nodeAnims)const=0;
	
	//! \brief Sets up the anim list to be used for loading in an xml animation file or putting nodes and their tracks into the mixer.
	
	//! \param[in] nodeTab Nodes you want to load
	//! \param[in] includePB2s Whether or not to include param block2 tracks when loading in the animation.
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks based upon the flags passed into the function.
	//! \see NodeAndAnims
	//! \see IMixer8::AddMaxMixer
	//! \see ILoadSaveAnimation::LoadAnimationNodeAnims
	virtual void SetUpAnimsForLoad(Tab<INode *> &nodeTab,BOOL includePB2s,	Tab<NodeAndAnims> &nodeAnims)const =0;

	//! \brief Sets up the anim list to be used for mapping an xml animation file.
	
	//! \param[in] nodeTab Nodes you want to map
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::CreateMapFileNodeAnims
	virtual void SetUpAnimsForMap(Tab<INode *> &nodeTab,	Tab<NodeAndAnims> &nodeAnims)const =0;
	//@}
	
	/*! \internal !*/
	virtual void LoadForMixing(BOOL flag) const =0;
};

//! Helper method for getting the ILoadSaveAnimation10 interface
#define GetILoadSaveAnimation10()	static_cast<ILoadSaveAnimation10*>(GetCOREInterface(ILOADSAVEANIMATION_INTERFACE))

//! \brief An extension to ILoadSaveAnimation that exposes additional functionality.

//! This class contains key functionality for saving and creating both animation and map files
//! and for loading in animation.
//! You get this interface by calling GetILoadSaveAnimation10()
class ILoadSaveAnimation10 : public ILoadSaveAnimation
{
public:

	//! \brief  Creates a list of NodeAndAnims items to be used for saving animation. Use as a replacement for 
	//! ILoadSaveAnimation::SetUpAnimsForSave

	//! \param[in] nodeTab Nodes whose tracks you want to save out.
	//! \param[in] saveFlag Determines which tracks under the specified nodes will be present in the
	//! resulting nodeAnims tab.  The possible values of the saveFlag are found in the ILoadSaveAnimation::SaveFlags enum.
	//! The possible vales are eAnimatedTracks, if this value is set, then only animated tracks only will be saved, 
	//! otherwise non-animated tracks will also be saved. eIncludeConstraints, if set and eAnimatedTracks is set, 
	//! then constraints will also be saved out, otherwise if eAnimatedTracks is set and this value isn't set, 
	//! they won't be saved out.  If eAnimatedTracks isn't set this parameter is unused since constraints will be 
	//! automatically saved out. eKeyableTracks, if set keyable tracks only will be saved, otherwise non-keyable tracks 
	//! will also be saved. Not that we don't use the eSaveSeg or eSegPerFrame flags with this function
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks based upon the flags passed into the function.  This
	//! can then be passed to ILoadSaveAnimation::SaveAnimationNodeAnims in order to save the animation.
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::SaveAnimationNodeAnims
	virtual void SetUpAnimsForSaveExt(const Tab<INode*> &nodeTab, DWORD saveFlag, Tab<NodeAndAnims> &nodeAnims)const =0;
	
	//! \brief Creates a list of NodeAndAnims items to be used for saving animation based upon a specified list of TrackViewPick items.  Use this
	//! function instead of ILoadInterface10::SetUpAnimsForSaveExt when you want to save animation from specific tracks.

	//! \param[in] res  A Tab of TrackViewPick items, each of which specifies a single animatable track, usually from a TrackViewPick dlg. \see TrackViewPick
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks.  This can then be passed to ILoadSaveAnimation::SaveAnimationNodeAnims
	//! in order to save the animation.
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::SaveAnimationNodeAnims
	virtual void SetUpAnimsForSaveFromTracks(const Tab<TrackViewPick> &res,Tab<NodeAndAnims> &nodeAnims)const =0;

	//! \brief Creates a list of NodeAndAnims items to be used for loading animation or for putting tracks into the mixer.

	//! \param[in] nodeTab Nodes whose tracks you want to load onto.
	//! \param[in] loadFlag Determines which tracks under the specified nodes will be loaded onto.  The possible values
	//! of the loadFlag are found in the ILoadSaveAnimation::LoadFlags enum. The possible vales are:  eStripLayers, if this value is set, 
	//! then any layer controller name will be removed from the incoming controller names, otherwise it won't. eIncludePB2s,
	//! if set then we will include any ParamBlock2 tracks.  Note that we don't use the eRelative or eInsert flags.
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks based upon the flags passed into the function. This can then be
	//! sent to either ILoadSaveAnimation::LoadAnimationNodeAnims or IMixer8::AddMaxMixxer.
	//! \see NodeAndAnims
	//! \see IMixer8::AddMaxMixer
	//! \see ILoadSaveAnimation::LoadAnimationNodeAnims
	virtual void SetUpAnimsForLoadExt(const Tab<INode *> &nodeTab,DWORD loadFlag,	Tab<NodeAndAnims> &nodeAnims)const =0;

	//! \brief Creates a list of NodeAndAnims items to be used for loading animation based upon a specified list of TrackViewPick items.  Use this
	//! function instead of ILoadInterface10::SetUpAnimsForLoadExt when you want to load animation upon specific tracks.

	//! \param[in] res  A Tab of TrackViewPick items, each of which specifies a single animatable track, usually from a TrackViewPick dlg. \see TrackViewPick
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks.  This can then be sent to either ILoadSaveAnimation::LoadAnimationNodeAnims
	//! or IMixer8::AddMaxMixer
	//! \see NodeAndAnims
	//! \see IMixer8::AddMaxMixer
	//! \see ILoadSaveAnimation::LoadAnimationNodeAnims	
	virtual void SetUpAnimsForLoadFromTracks(const Tab<TrackViewPick> &res, Tab<NodeAndAnims> &nodeAnims)const =0;

	//! \brief Creates a list of NodeAndAnims items to be used for mapping animation.

	//! \param[in] nodeTab Nodes whose tracks you want to map onto.
	//! \param[in] mapFlag Determines which tracks under the specified nodes will be mapped onto.  The possible values
	//! of the mapFlag are found in the ILoadSaveAnimation::MapFlags enum. The possible vales are:  eStripLayersInMap, if this value is set, 
	//! then any layer controller name will be removed from the incoming controller names, otherwise it won't.  Note that we don't
	//! use eMapControllerExactName or eMapControllerType flags.
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks based upon the flags passed into this function.  This can
	//! then be sent to ILoadSaveAnimation::CreateMapFileNodeAnims.
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::CreateMapFileNodeAnims
	virtual void SetUpAnimsForMapExt(const Tab<INode *> &nodeTab,DWORD mapFlag,	Tab<NodeAndAnims> &nodeAnims)const =0;

	
	//! \brief Creates a NodeAndAnims items to be used for mapping animation based upon a list of TrackViewPick items.   Use this
	//! function instead of ILoadInterface10::SetUpAnimsForMapExt when you want to map animation from specific tracks.

	//! \param[in] res  A Tab of TrackViewPick items, each of which specifies a single animatable track, usually from a TrackViewPick dlg. \see TrackViewPick
	//! \param[out] nodeAnims The returned list of nodes and their anim tracks.  This can then be sent to ILoadSaveAnimation::CreateMapFileNodeAnims.
	//! \see NodeAndAnims
	//! \see ILoadSaveAnimation::CreateMapFileNodeAnims
	virtual void SetUpAnimsForMapFromTracks(const Tab<TrackViewPick> &res,Tab<NodeAndAnims> &nodeAnims)const =0;

	
	//! \brief Bring up a load animation dialog that will load animation to the each track that's specified in the tab of TrackViewPick items.

	//! \param[in] res  A Tab of TrackViewPick items, each of which specifies a single animatable track, usually from a TrackViewPick dlg. \see TrackViewPick
	//! \return  The name of the file if a file gets loaded, otherwise returns NULL. 
	virtual const MCHAR * LoadAnimationDlgFromTracks(const Tab<TrackViewPick> &res)const = 0;

	//! \brief Bring up a save animation dialog that will save animation from the each track that's specified in the tab of TrackViewPick items.

	//! \param[in] res  A Tab of TrackViewPick items, each of which specifies a single animatable track, usually from a TrackViewPick dlg. \see TrackViewPick
	//! \return  The name of the file if a file gets saved, otherwise returns NULL.
	virtual const MCHAR * SaveAnimationDlgFromTracks(const Tab<TrackViewPick> &res)const =0;


	//! \brief Given a specified map file, it will replace all specified string attributes with the new string attributes,
	//! and a create a new map file with those new strings.

	//! \param[in] mapFile  The name of the map file that you want to search and replace attribute strings from.
	//! \param[in] newMapFile The name of the new map file that will be created with the replaced attribute strings.
	//! \param[in] oldStrings  The list of old string values you wish to replace
	//! \param[in] newStrings  The list of new string values that will replace the corresponding old string value.
	//! \param[in] mapName If true it will replace the 'mapName' attribute in the map xml file, otherwise it will change the 'name' attribute value.
	//! \return  TRUE if we had strings to replace and we were able to create the new map file. If we return FALSE, no new map file
	//! was created.
	virtual BOOL ReplaceAttributesInMapFile(const MCHAR *mapFile,const MCHAR *newMapFile,Tab<MSTR*> &oldStrings, Tab<MSTR*> &newStrings,BOOL mapName) const =0;

	//! \brief Given a set of nodes it will create a map file that maps the nodes tracks onto themselves, which is the same as doing 'default' mapping.

	//! \param[in] nodeTab Nodes you want to map to themselves.
	//! \param[in] mapFile The name of the map file you to create with the default mapping.
	//! \return  TRUE if the map file is created, FALSE if for some reason we couldn't create the map file.
	virtual BOOL CreateDefaultMapFile(Tab<INode *> &nodeTab,const MCHAR *mapFile)const = 0;
};




