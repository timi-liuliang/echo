/*==============================================================================
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        XMLAnimTreeEntry.h
// DESCRIPTION: Interface to Load Save Animation Interfaces
// AUTHOR:      Michael Zyracki 
// HISTORY:     created 2005
//***************************************************************************/
#pragma once

#include "export.h"
#include "ifnpub.h"
#include "ref.h"

//! \name Expose World Class IDs
//@{
//! \brief Expose World Transform Super Class ID 

//! Special 'fake' class ID's used for the special exposed world TM.  This lets us put the world TM
//! of a node into the XMLAnimTreeList and have it act as if it's a controller
#define EXPOSE_WORLD_TRANSFORM_SCLASS_ID 0x7f765e27
//! brief Expose World Transform Class ID

//! Special 'fake' class ID's used for the special exposed world TM.  This lets us put the world TM
//! of a node into the XMLAnimTreeList and have it act as if it's a controller
#define EXPOSE_WORLD_TRANSFORM_CLASS_ID Class_ID(0x7f765e27, 0x13e613a0) 
//@}

 

class XMLAnimTreeEntryList;
class XMLAnimLabelWindow;
class XMLAnimMapDlg;
class AnimFileLoader;
class XMLAnim;
class AnimFileCreator;

//! Interface ID for the XMLAnimTreeEntry FPMixinInterface.
#define XMLANIMTREEENTRY_INTERFACE  Interface_ID(0x5d25abd, 0x36934bff)

//! \brief XMLAnimTreeEntry class that's used for storing a controller track

//! Class used to specify a controller/animation track that is then used by the load save and mixer systems.
//! They will reside in the XMLAnimTreeEntryList.
//! \see XMLAnimTreeEntryList
class XMLAnimTreeEntry : public FPMixinInterface
{
	/*! \internal !*/
	friend class XMLAnimTreeEntryList;
	/*! \internal !*/
	friend class XMLAnimLabelWindow;
	/*! \internal !*/
	friend class XMLAnimMapDlg;
	/*! \internal !*/
	friend class AnimFileLoader;
	/*! \internal !*/
	friend class AnimFileCreator;
	/*! \internal !*/
	friend class XMLAnim;
	/*! \internal !*/
	friend class XMLTreeListEnum;
	/*! \internal !*/
	friend class XMLAnimTreeEntryImp;

public:
	//! \brief Enum of type that a controllery can be, e.g position track, rotation y track, note track etc..
	
	//! Enum of type that they can be, The can be more than one type. eNone means it has no type,
	//! eTransform means that the track is a transform controller or a child of one, 
	//! eBaseObject means that the track is under an object, eModObject means the track is under a modifier,
	//! eNoteTracks means the track is a NoteTrack, eVisTracks means the track is a visibility track, eMatMaps
	//! means the track is under a material map, eMatParams means the track is under a material parameter,
	//! eCustomAttributes means the track is under a custom attribute, eIK means the track is under an IK controller,
	//! ePos means that track is position controller or one of it's children, eRot means the track is a
	//! rotation controller or one of it's children, eScale means the track is a scaleController or one of it's children,
	//! ePosX means the track is a position x controller, ePosY means the track is a position y controller, ePosZ
	//! means the track is a position z controller, eList means the track is a list controller or one of it's children,
	//! eRotX means the track is a  rotation x controller, eRotY means the track is a rotation y controller, eRotZ
	//! means the track is a rotation z controller, eKeyable means the track is keyable,eScaleX means the track is a scale
	//! controller, eScaleY means the track is a scale y controller, eScaleZ means the track is a scale z controller,
	//! eExposeWorldTransform means the track is the world TM of a node,eLayer means that the track is a layer controller
	//! or a child of one
	enum Type {eNone = 0x0, eTransform = 0x1, eBaseObject = 0x2, eModObject = 0x4, eNoteTracks = 0x8, 
	eVisTracks = 0x10, eMatMaps = 0x20,	eMatParams = 0x40, eCustomAttributes = 0x80, 
	eIK = 0x100, ePos = 0x200,eRot =0x400, eScale = 0x800, ePosX = 0x1000,
	ePosY = 0x2000, ePosZ = 0x4000,eList = 0x8000,eKeyable = 0x10000,eRotX = 0x20000,eRotY = 0x40000,eRotZ = 0x80000,
	eScaleX = 0x100000,eScaleY = 0x200000,eScaleZ = 0x400000,eExposeWorldTransform = 0x800000,eLayer = 0x1000000};

	//! \brief The animatable track.
	//! \return The animatable track
	DllExport Animatable *GetAnim()const {return anim;}
	
	//! \brief The animatable track's parent.
	//! \return The animatable track's parent
	DllExport Animatable *GetClient() const{return client;}
	
	//! \brief The sub anim number of the anim under the client.
	//! \return The sub anim number of the anim under the client
	DllExport int GetSubNum()const{return subNum;}
	
	//! \brief The name of the track. This plus the unique identifier is used to uniquely identify a track.
	//! \return The name of the track. This plus the unique identifier is used to uniquely identify a track
	DllExport const MCHAR *GetName()const {return name.data();}
	
	//! \brief Number used to specify how many other controllers exist before this one in the XMLAnimTreeEntryList with the same name.
	//! \return The number used to specify how many other controllers exist before this one in the XMLAnimTreeEntryList with the same name
	DllExport int GetUnique()const {return unique;}
	
	//! \brief The name of the track that this track is mapped to
	//! \return The name of the track that this track is mapped to
	DllExport const MCHAR *GetMapName()const{return  mapName.data();}
	
	//! \brief Unique number for the track that it is mapped to
	//!\return The unique number for the track that it is mapped to.
	DllExport int GetMapUnique()const {return mapUnique;}

	//! \brief Returns true if the object is the passed in type  \see XMLAnimTreeEntry::Type
	//! \param[in] type The type to check against
	//! \return True if that type, false otherwise
	DllExport bool IsType(int type) const;
	
	//! \brief The possible type of the anim track as defined in the enum Type definition.
	
	//! Stored as an int instead of as an enum in order to speed up comparison operations.
	//! \return The type
	DllExport unsigned int GetType()const{return type;}

	/*! \internal !*/
	enum { getAnim,getClient,getSubNum,getName,getUnique,getMapName,getMapUnique,isType};
	// Function Map
	/*! \internal !*/
	BEGIN_FUNCTION_MAP
		/*! \internal !*/
		RO_PROP_FN(getAnim, GetAnimRef, TYPE_REFTARG);
		/*! \internal !*/
		RO_PROP_FN(getClient, GetClientRef, TYPE_REFTARG);
		/*! \internal !*/
		RO_PROP_FN(getSubNum, GetSubNum, TYPE_INT);
		/*! \internal !*/
		RO_PROP_FN(getName, GetName, TYPE_STRING);
		/*! \internal !*/
		RO_PROP_FN(getUnique, GetUnique, TYPE_INT);
		/*! \internal !*/
		RO_PROP_FN(getMapName, GetMapName, TYPE_STRING);
		/*! \internal !*/
		RO_PROP_FN(getMapUnique, GetMapUnique, TYPE_INT);
		/*! \internal !*/
		FN_1(isType, TYPE_bool, IsType,TYPE_ENUM);	
	
	END_FUNCTION_MAP

	//for FP 
	/*! \internal !*/
	DllExport FPInterfaceDesc* GetDesc();
	/*! \internal !*/
	LifetimeType LifetimeControl() { return wantsRelease; }
	/*! \internal !*/
	BaseInterface* AcquireInterface() { return this; }
	/*! \internal !*/
	DllExport void ReleaseInterface();

private:
	
	DllExport   XMLAnimTreeEntry(Animatable *a, Animatable *c,int s, int d);
	DllExport 	XMLAnimTreeEntry(const XMLAnimTreeEntry &entry);

	Animatable *anim;
	Animatable *client;
	int subNum;
	int depth;
	MSTR name; 
	int unique;
	MSTR mapName;
	int mapUnique;
	unsigned int type;
	DWORD flags;
	BOOL selected,mapped; //used in XMLAnimWindow
	int location; //used to specify where it came from the original list, in XMLAnimWindow
	BOOL loadable;//used when loaded in and a child is being loaded, we need to set all the parent/anims, as unloadable
	MSTR className; //name of it's class
	SClass_ID superClassID;
	Class_ID classID;
	MSTR mapClassName;
	MSTR nodeParentName; //only used for nodes!!!!
	MSTR IKBeginNode; //only used for IKControlChain Nodes.
	MSTR IKEndNode;
	MSTR IKSplineNode; //only used by nodes that are dummies that control a spline IK. This is the IKChainControl node
	float distanceToParent;//only fo nodes.
	int numChildren; //subanims or node children


	DllExport int GetParentIndex(XMLAnimTreeEntryList *list, int listIndex);
	DllExport unsigned int GetLocalType();
	DllExport unsigned int IsXYZ();
	DllExport BOOL IsList();
	DllExport bool IsParentOpen(XMLAnimTreeEntryList *list, int listIndex);
	DllExport MSTR StripNodeFromName(); //TODO make sure we handle nodeless entries


	//FP functions and info
	DllExport ReferenceTarget* GetAnimRef() {return dynamic_cast<ReferenceTarget*>(anim);} // This needs to be deinlined.
	DllExport ReferenceTarget* GetClientRef() {return dynamic_cast<ReferenceTarget*>(client);} // This needs to be deinlined.

};



//! Interface ID for the XMLAnimTreeEntryList class.
#define XMLANIMTREEENTRYLIST_INTERFACE  Interface_ID(0x34d56bf5, 0x1b2a7c80)


//! \brief The list of XMLAnimTreeEntry tracks, that will then be loaded onto, or saved out, or put into the mixer.

//! The class is contains pointers to the XMLAnimTreeEntry objects and allocates and deletes these objects itself.
//! This class is contained within the NodeAndAnims object.
//! \see XMLAnimTreeEntry
//! \see NodeAndAnims
class XMLAnimTreeEntryList : private Tab<XMLAnimTreeEntry*>,public FPMixinInterface		
{
	/*! \internal !*/
	friend class XMLAnimTreeEntry;
	/*! \internal !*/
	friend class XMLAnimLabelWindow;
	/*! \internal !*/
	friend class XMLAnimMapDlg;
	/*! \internal !*/
	friend class AnimFileLoader;
	/*! \internal !*/
	friend class AnimFileCreator;
	/*! \internal !*/
	friend class XMLAnim;	
	/*! \internal !*/
	friend class XMLTreeListEnum;
	/*! \internal !*/
	friend class XMLAnimTreeEntryListImp;
public:	

	//! \brief Constructor
	DllExport XMLAnimTreeEntryList();
	//! \brief Deconstructor
	DllExport ~XMLAnimTreeEntryList();


	//! \brief Returns the i'th XMLAnimTreeEntry
	
	//! \param[in] index The index of the item you want
	//! \return The item for tha index, NULL if the index is out of range
	DllExport XMLAnimTreeEntry* operator[](const int index);

	//! \brief How many items in the list
	//! \return The number of items in the list
	DllExport int Count() const;
	
	//! \brief Zeroes out the list and deletes the XLMAnimTreeEntries in the list.
	DllExport void ZeroCount();
	
	//! \brief Shrinks the internal Tab that holds the items.
	DllExport void Shrink();
	
	//! \brief Delete the specified items out of the list. Also deletes the XMLAnimTreeEntry objects themselves.

	//! \param[in] start The first item to delete
	//! \param[in] num The number of items to delete from the start.
	//! \return The number of items left in the list
	DllExport int Delete(int start,int num) ;
	
	//! \brief Deletes all of the XMLAnimTreeEntries in the list
	DllExport void DeleteData();

	//! \brief Find the specified anim in the list.

	//! \param[in] anim The anim you want to find.
	//! \param[in] start Where you want to start looking from
	//! \return The index where the entry is, -1 if not found
	DllExport int FindAnim(Animatable *anim,int start=0);

	//! \brief Find this particular XMLAnimTreeEntry based off it's pointer value.
	
	// !\param[in] which XMLAnimTreeEntry you want to find
	// !\return The index where the entry is, -1 if not found
	DllExport int Find(XMLAnimTreeEntry *which);

	// \brief Find this particular XMLAnimTreeEntry based off it's name and unique id.
	
	// !param[in] name The name of the track you want to find
	// !param[in] unique Whether or not the track is unique
	// !\return The index where the entry is, -1 if not found
	DllExport int Find(MSTR &name, int unique);

	// !\brief Find the subAnim parents of the specified item.

	//! \param[in] index The index of the items subAnim parents you want to find.
	//! \param[out] parentIndices The indices of the items parents.  It will be empty if it has no subAnim parents.
	DllExport void FindParents(int index,Tab<int> &parentIndices);

	//! \brief Fnd the subAnim children of the specified item.

	//! \param[in] index The index whose children you want to find.
	//! \param[out] childIndices The indices of the items children.  It will be empty if it has no subAnim children.
	DllExport void FindChildren(int index,Tab<int> &childIndices);


	//FP stuff
	/*! \internal !*/
	enum { getEntry,count,zeroCount,shrink,deleteItems,findAnim,findName};
	// Function Map
	/*! \internal !*/
	BEGIN_FUNCTION_MAP  
	/*! \internal !*/
		FN_1(getEntry, TYPE_INTERFACE, GetEntry,TYPE_INT);
	/*! \internal !*/
		FN_0(count,TYPE_INT,Count);
	/*! \internal !*/
		VFN_0(zeroCount,ZeroCount);
	/*! \internal !*/
		VFN_0(shrink,Shrink);
	/*! \internal !*/
		FN_2(deleteItems,TYPE_INT,Delete,TYPE_INT,TYPE_INT);
	/*! \internal !*/
		FN_2(findAnim,TYPE_INT,FindAnimFP,TYPE_REFTARG,TYPE_INT);
	/*! \internal !*/
		FN_2(findName,TYPE_INT,FindNameFP,TYPE_STRING,TYPE_INT);
	
	END_FUNCTION_MAP

	//for 
	/*! \internal !*/
	DllExport FPInterfaceDesc* GetDesc();
	/*! \internal !*/
	LifetimeType LifetimeControl() { return wantsRelease; }
	/*! \internal !*/
	BaseInterface* AcquireInterface() { return this; }
	/*! \internal !*/
	DllExport void ReleaseInterface();

	
private:
	//fp stuff
	FPInterface* GetEntry(const int index);
	int FindAnimFP(ReferenceTarget *anim,int start);
	int FindNameFP(MCHAR *name,int unique);

	int curSel;
	DllExport void Build(	Animatable *root);
	DllExport MSTR GetName(XMLAnimTreeEntry *entry, int listIndex);
	DllExport void SetNameTypeUnique(XMLAnimTreeEntry* entry,int index);

	DllExport void DeleteSubtree(int index);
	DllExport void RecalcSubtree(int index,DWORD flags=0);

	DllExport void AppendEntry(XMLAnimTreeEntry &entry);
	DllExport void InsertEntry(int where,XMLAnimTreeEntry &entry);

	DllExport BOOL IsTextSelected(int index);
	DllExport void ClearSelectText();

	DllExport void GetSelected(Tab<XMLAnimTreeEntry*> &selected);
	DllExport void GetAll(Tab<XMLAnimTreeEntry*>&all);
	DllExport void SelectByWildCard(MCHAR *buf);
	DllExport void SelectText(int index,BOOL sel,BOOL range=FALSE,BOOL column=FALSE);
	DllExport int FindSelectText();
	DllExport void SelectItemText(int index);
	DllExport void SelectAllText();
	DllExport void SelectInvertText();
	DllExport void SelectChildrenText(int index);

	DllExport void AddWorldTransformEntry(MSTR &nodeName,int where);

	DllExport int LineCount();
	DllExport int MaxDepth();

	DllExport int EntryRealDepth(int i);
	DllExport int RealMaxDepth();
	DllExport void MarkParentsFromListAsUnloadable(int index);
	DllExport void MarkParentsAndChildrenAsMapped(BOOL mapped,int index);
	DllExport void MarkParentsAndParentsChildrenAsMapped(BOOL mapped,int index);
	DllExport bool AreAnyChildrenMapped(int index);

	DllExport void FindAllParentNodes(int index,Tab<int> &nodeParents); //find all of the parent nodes
	DllExport void FindAllChildNodes(int index,Tab<int> &nodeChildren); //find all of the child nodes
	DllExport void FindChildNodes(int index,Tab<int> &nodeChildren); //find just the direct child nodes
	DllExport void SetMapInfo(BOOL mapped,int which, MSTR &mapName, int mapUnique);

};

//! Interface ID for the NodeAnims FPMixinInterface.
#define NODEANDANIMS_INTERFACE  Interface_ID(0x4ad13c9b, 0x3ca97bec)

//! \brief A class that holds a node and a list of anims under that node that can be used for loading,saving, mapping animation, or
//! putting specific tracks into the mixer. Provides a lower level of control than just specify a node by itself.

//! This class gets filled out via the ILoadSaveAnimation::SetUpAnimsForSave, ILoadSaveAnimation::SetUpAnimsForLoad,
//! and ILoadSaveAnimation::SetUpAnimsForMapping. It's the users responsibility to delete this item by calling DeleteThis, since it doesn't
//! delete it's own data on deconstruction.
//! \see ILoadSaveAnimation::SetUpAnimsForSave
//! \see ILoadSaveAnimation::SetUpAnimsForLoad
//! \see ILoadSaveAnimation::SetUpAnimsForMap
//! \see IMixer8::AddMaxMixer
//! \see XMLAnimTreeEntryList

class NodeAndAnims : public FPMixinInterface
{
	/*! \internal !*/
	friend class XMLAnimLabelWindow;
	/*! \internal !*/
	friend class XMLAnimMapDlg;
	/*! \internal !*/
	friend class AnimFileLoader;
	/*! \internal !*/
	friend class AnimFileCreator;
	/*! \internal !*/
	friend class XMLAnim;	
	/*! \internal !*/
	friend class NodeAndAnimsImp;
	/*! \internal !*/
	friend class ILoadSaveAnimation_Imp;
public:
	NodeAndAnims():node(NULL),IKNodeName(NULL),list(NULL){};

	//! \brief Copy constructor
	DllExport NodeAndAnims(const NodeAndAnims &);
	
	//! \brief Get the XMLAnimTreeEntry list
	//! \return The XMLAnimTreeEntryList object
	DllExport XMLAnimTreeEntryList *GetList()const {return list;}
	
	//! \brief Get the node
	//! \return The node
	DllExport INode *GetNode()const {return node;}
	
	//! \brief Deletes the data held by the NodeAnim, in particular the list object. Must be called once the list is done being used.
	DllExport void DeleteThis();

	/*! \internal !*/
	enum { getList,getNode};
	//! Function Map
	#pragma warning(push)
	#pragma warning(disable:4100)
	BEGIN_FUNCTION_MAP  
	/*! \internal !*/
		FN_0(getList, TYPE_INTERFACE,GetListFP);
	/*! \internal !*/
		FN_0(getNode,TYPE_INODE,GetNode);
	END_FUNCTION_MAP
	#pragma warning(pop)
	// function map use
	/*! \internal !*/
	DllExport FPInterfaceDesc* GetDesc();
	/*! \internal !*/
	LifetimeType LifetimeControl() { return wantsRelease; }
	/*! \internal !*/
	BaseInterface* AcquireInterface() { return this; }
	/*! \internal !*/
	DllExport void ReleaseInterface();

private:

	INode *node; //node may be NULL!
	MSTR *IKNodeName;//name of the IK Node that this node may implicitly control, e.g a spline IK helper
	//node would have the name of the .. may be NULL!
	XMLAnimTreeEntryList *list;

	DllExport FPInterface *GetListFP();
};



