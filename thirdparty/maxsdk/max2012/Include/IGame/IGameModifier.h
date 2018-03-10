/**********************************************************************
 *<
	FILE: IGameModifier.h

	DESCRIPTION: Modifier interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameModifier.h
\brief IGame Modifier Interfaces including direct access skin,physique and the morphing based operators.

High level access to MAX's modifier, with specific exposure for Skin and Morph modifiers.  This includes the Morph compound object
*/

#pragma once

#include "IGameProperty.h"
#include "IConversionManager.h"
#include "IGameObject.h"
// forward declarations
class IGameProperty;
class IGameNode;
class Modifier;
class INode;

//!Simple wrapper for 3ds Max modifiers
/*! This is an IGame wrapper for the basic 3ds Max modifier class.  This is provided so that the developer does not need
to walk the modifier stack and look for derived objects.  An instance of this class is obtained from IGameObject class
\sa IGameObject
*/
class IGameModifier : public IExportEntity
{
private:
	Modifier * gameMod;
	INode * gameNode;
	MSTR intName;
public:

	//! IGame modifier types
	/*! These are the modifiers known to IGame
	*/
	enum ModType{
		IGAME_SKINNING,	/*!<A skinning Modifier*/
		IGAME_MORPHER,  /*!<A Morphing based Modifier/Object*/
		IGAME_GENERAL,	/*!<A generic 3ds Max modifier*/
	};

	//! The IGameModifier constructor
	/*!
	\param *mod  The 3ds Max modifier to initialise
	\param *node  The 3ds Max node to initialise
	*/
	IGameModifier(Modifier * mod, INode * node);


	//! Get IGameModifier type
	/*! Return the Type of Modifier IGameModifier represents
	\return The modifier type, corresponds to IGameModifier::ModType Enum
	*/
	virtual ModType GetModifierType() =0;
	
	//!Get the modifier Name
	/*!The name as viewed in StackView
	\return The name
	*/
	virtual MCHAR * GetUIName() ;

	//!Get the modifier Name
	/*!The internal name of the modifier
	\return The name
	*/
	virtual MCHAR * GetInternalName() ;

	//! Access to the 3ds Max modifier
	/*! This is provided so the developer can get to any LocalModData that may have been added to the modifier
	\return The pointer a standard max modifier. 
	*/
	virtual Modifier * GetMaxModifier();

	//! Access to the nodes this modifier is applied to
	/*! This enumerates all the nodes that are effected by this modifier. 
	\param &nodeList The tab to receive the node list.  This will always be at least 1 in size, as it will contain the 
	original node.
	*/
	virtual void EffectedNodes(Tab<INode*> &nodeList);

	//! Check whether the modifier is a skinning modifier
	/*!
	\return TRUE if the modifier is a skinning modifier
	*/
	virtual bool IsSkin();

	//! Check whether the modifier is the  morpher modifier
	/*!
	\return TRUE if the modifier is the morpher modifier
	*/
	virtual bool IsMorpher();

	virtual ~IGameModifier();

};


//! A skin wrapper Class
/*! This class provides an unified interface to the various skin options present in 3ds Max.  This includes Physique and Skin.
All the data from skin and physique are stored in the same way, but options exist to find out what skinning option was used.
The vertex indexes used here are the same as those for the actual mesh, so this provides a one to one corelation.
\n
The version of the Character Studio that is used for IGame is 3.2.1 - Anything earlier is unsupported
\n
The bones need to be parsed by IGame before this interface can be used.
\sa IGameModifier
*/

class IGameSkin : public IGameModifier
{
public:

	//! Skinning Modifier Types
	/*! These are the Skin modifiers known to IGame
	*/
	enum SkinType{
		IGAME_PHYSIQUE,	/*!<A Physique Modifier*/
		IGAME_SKIN,	/*!<A 3ds Max Skin Modifier*/
	};
	//! Vertex types
	/*! These are the types used by the modifiers 
	*/
	enum VertexType{
		IGAME_RIGID,			/*!<A RIGID vertex*/
		IGAME_RIGID_BLENDED,	/*!<A BLENED vertex*/
		IGAME_UNKNOWN			/*!<Error or unsupported vertex*/
	};


	//! Get the numbers of vertices effected by this instance of the modifier.  
	/*! If the modifier is attached to more than one node, then this will be the count of vertices on the current node
	\return The number of vertices
	*/
	virtual int GetNumOfSkinnedVerts()=0;
	
	//! Get the numbers of bones effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\return The number of bones
	*/
	virtual int GetNumberOfBones(int vertexIndex)= 0;
	
	//! Get the weight for the bone and vertex index passed in
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return The weight
	*/
	virtual float GetWeight(int vertexIndex,int boneIndex) = 0;

	//! Get the 3ds Max bone effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A pointer to a 3ds Max INode for the bone
	*/
	virtual INode * GetBone(int vertexIndex,int boneIndex)= 0;
	
	//! Get the IGameNode equivalent of the bone effecting the vertex
	/*!
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A pointer to a IGameNode for the bone
	*/
	virtual IGameNode * GetIGameBone(int vertexIndex,int boneIndex)= 0;

	//! Get the IGameNode ID equivalent of the bone effecting the vertex
	/*! The IGameNode ID can be used if the nodes pass out  first
	and use this value as an index when upon import
	\param vertexIndex The index of the vertex
	\param boneIndex The bone index 
	\return A Node ID
	*/
	virtual int GetBoneID(int vertexIndex, int boneIndex) =0;
	
	//! Get Vertex Type 
	/*! Specifies whether the Vertex is either Rigid or blended
	\param vertexIndex The vertex to query
	\return The vertex type.  It can be one of the following\n
	IGAME_RIGID\n				
	IGAME_RIGID_BLENDED\n
	\sa IGameSkin::VertexType
	*/
	virtual VertexType GetVertexType(int vertexIndex)=0;
	
	//! Get the Skinning type
	/*! This can be used to find out whether 3ds Max's Skin or Physique was used
	\return The skinning type.  It can be one of the following\n
	IGAME_PHYSIQUE\n	
	IGAME_SKIN\n
	\sa IGameSkin::SkinType
	*/
	virtual SkinType GetSkinType()=0;

	//! Get the bone TM when skin was added
	/*! This provides access to the intial Bone TM when the skin modifier was applied.
	\param boneNode THe IGameNode bone whose matrix is needed
	\param &intMat  THe matrix to receive the intial TM
	\return True if the bone was found
	*/
	virtual bool GetInitBoneTM(IGameNode * boneNode, GMatrix &intMat)=0;

	//! Get the bone TM when skin was added
	/*! This provides access to the intial Bone TM when the skin modifier was applied.
	\param boneNode The Max INode bone whose matrix is needed
	\param &intMat  The matrix to receive the intial TM
	\return True if the bone was found
	*/
	virtual bool GetInitBoneTM(INode * boneNode, GMatrix &intMat)=0;

	//! Get the original TM for the node with skin. 
	/*! This provides access to the intial node TM when the skin modifier was applied.
	\param &intMat  The matrix to receive the intial TM
	*/
	virtual void GetInitSkinTM(GMatrix & intMat) = 0;

	//! Get the number of bones used by the skinning modifiers
	/*! This provides the toal number of bones used by the modifiers.  This shouldn't be confused with IGameSkin::GetNumberOfBones, which
	returns the numbers of bones associated with a vertex.
	\returns The total number of bones in the system
	*/
	virtual int GetTotalBoneCount()=0;

	//! Receive the total number of bones used by the skinning system - this will include bones that are not assigned to any vertex
	virtual int GetTotalSkinBoneCount() = 0;

	//! Returns the mesh before the skin modifier was added.
	virtual IGameMesh * GetInitialPose()=0;

	//! use the usedList to define if you want to the search all the bones or just the bones actually effecting a vertex
	virtual int GetBoneIndex(IGameNode * boneNode, bool usedList = false) =0;

	//! use the usedList to define if you want to the search all the bones or just the bones actually effecting a vertex
	virtual int GetBoneIndex(INode * boneNode , bool usedList = false)=0;

	//! use the usedList to define if you want to the search all the bones or just the bones actually effecting a vertex
	virtual IGameNode * GetIGameBone(int index, bool usedList = false)=0;

	//! use the usedList to define if you want to the search all the bones or just the bones actually effecting a vertex
	virtual INode * GetBone(int index, bool usedList = false) = 0;

	//! The IGameSkin constructor
	/*!
	\param *mod  The 3ds Max modifier to initialise
	\param *node  The 3ds Max node to initialise
	*/
	IGameSkin(Modifier * mod, INode * node):IGameModifier(mod, node){};



};


//! An IGame wrapper around the Morph Compound Object and the Morpher Modifier
class IGameMorpher : public IGameModifier
{
public:
	//! A morph type 
	/*! This provides details about the type of morph operator being accessed
	*/
	enum MorphType{
			MORPHER_OBJECT,		/*!<A Morph Compound Object*/
			MORPHER_MODIFIER,	/*!<A Morph Modifier*/
	};

	//! Get the number of morph targets
	/*! The number of targets used by the morpher.  There is a slight difference between the morph methods.  The compound
	morpher include the original object as a target, where as the morpher modifier only show the actual physical targets. So
	to provide the same result the compound will show one more target
	\returns The number of morph targets
	*/
	virtual int GetNumberOfMorphTargets() = 0;

	//! Get the morph target used by the morpher
	/*! This method provides the actual morph target used by the system.  In the case of the compound object, it does its best to find the node
	but, the compound morpher only exposes the actual "Object" and in 3ds Max the object could have multiple nodes.
	\param index The index of the target to retrieve
	\returns An IGameNode representing the target
	*/
	virtual IGameNode * GetMorphTarget(int index) = 0;

	//! Get the weight associated with the target
	/*!
	\param index The index of the target whose weight is being accessed
	\returns An IGameControl.  The actual value of the keys provides the weight value - this value can not be consider normalised
	*/
	virtual IGameControl * GetMorphWeight(int index)= 0;

	//! Get the morpher type
	/*! The type of morpher represented by this interface
	\return The morpher type.  It can be one of the following\n
	MORPHER_OBJECT \n
	MORPHER_MODIFIER\n
	\sa IGameMorpher::MorphType
	*/
	virtual MorphType GetMorphType() = 0;

	//! The IGameMorpher constructor
	/*!
	\param *mod  The 3ds Max modifier to initialise
	\param *node  The 3ds Max node to initialise
	*/
	IGameMorpher(Modifier * mod, INode * node):IGameModifier(mod, node){};
};

//! A generic Modifier class
/*! Any modifier that is not known to IGame will be implemented as a "Generic" modifier, so that basic access can be provided
*/
class IGameGenMod : public IGameModifier
{
public:
	
	//! The IGameGenMod constructor
	/*!
	\param *mod  The 3ds Max modifier to initialise
	\param *node  The 3ds Max node to initialise
	*/
	IGameGenMod(Modifier * mod, INode * node):IGameModifier(mod, node){};

};


