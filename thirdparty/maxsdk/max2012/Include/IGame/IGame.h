/**********************************************************************
 *<
	FILE: IGame.h

	DESCRIPTION: Scene and Node interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGame.h
\brief Main IGame scene access including node and materials
*/
#pragma once

#include "..\maxheap.h"
#include "IGameControl.h"
#include "IGameMaterial.h"
#include "IGameObject.h"
#include "IGameStd.h"


//!A simple wrapper for max nodes
/*!
IGameNode provides simplified access to many of 3ds Max's standard Node.  The user can use this class to gain access
to Controllers and Objects, plus many basic node properties.  An instance of this class is available through the IGameScene
interfaces.
\sa IGameScene
\sa IGameObject
*/

class IGameNode: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IGameNode() {;}

	//! Get the IGameControl Interface 
	/*! The IGameControl provides access to the key frame data for the node
	\return An IGameControl pointer.
	*/
	virtual  IGameControl * GetIGameControl()  =0;
	
	//! Get the actual object
	/*! The is the object used by IGame, all the basic objects supported by IGame derive from IGameObject 
	IGameObject::GetIGameType should be used to determine how to cast the returned pointer.
	\return An IGameObject pointer
	*/
	virtual IGameObject * GetIGameObject()=0;

	//! Release the IGameObject obtained from GetIGameObject
	/*! This will release all the memory used by the object, but it will not effect any controller or transfrom
	data.  Remember to call GetIGameObject if you want to use it again.
	\n Starting with IGame v 1.2 reference counter used internally, so IGameObject will be released only in case of 
	matching GetIGameObject/ReleaseIGameObject calls.
	*/
	virtual void ReleaseIGameObject()=0;
	
	//! Get the node name
	/*! The name of the node as used in max
	\return The name of the object
	*/
	virtual  MCHAR * GetName()  = 0;
	
	//! Get the unique Node ID
	/*! This provideds access to the node handle used in 3ds Max.  This value will be unique for the file
	\return The node ID (node handle)
	*/
	virtual int GetNodeID() =0;

	//! Access to the 3ds Max node, if further processing is required
	/*! if need be the node can be accessed here - this is so that IGame can be intergrated into existing pipelines
	\return A pointer to a Max node
	*/
	virtual INode * GetMaxNode()=0;
	
	//! Get World TM
	/*!
	\param t the Time to retrieve the WorldTM.  If no time is passed in then the global static frame is used
	\return A matrix containing the World TM
	*/
	virtual GMatrix GetWorldTM(TimeValue t=TIME_NegInfinity)=0;

	//! Get Local TM
	/*!
	\param t the Time to retrieve the LocalTM.  If no time is passed in then the global static frame is used
	\return A Matrix containing the local TM
	*/
	virtual GMatrix GetLocalTM(TimeValue t=TIME_NegInfinity)=0;

	//! Get Object TM
	/*!
	\param t the Time to retrieve the ObjectTM.  If no time is passed in then the global static frame is used
	\return A Matrix containing the Object TM.  This is the TM after any world space transforms have been applied
	*/
	virtual GMatrix GetObjectTM(TimeValue t=TIME_NegInfinity)=0;

	//! Get the nodes parent
	/*! If this is a top level node, then it will not have any parents
	\returns If there is a parent a valid pointer will be returned, else NULL
	*/
	virtual IGameNode * GetNodeParent()  =0;

	//! Get the number of direct children to the parent
	/*! This does not include children of children
	\return The number of children
	*/
	virtual int GetChildCount()=0;

	//! Access the n'th child node of the parent node
	/*!
	\param index The index to the child to retrieve
	\return IGameNode pointer to the child
	*/
	virtual  IGameNode * GetNodeChild(int index) =0;

	//! Get the material index
	/*! The index into the material array for this node's material
	\return The index of material used by this node
	*/
	virtual int GetMaterialIndex()  =0;
	
	//! Get the material
	/*! Direct access to the actual material used by the node
	\return The IGameMaterial pointer for this node.
	*/
	virtual  IGameMaterial * GetNodeMaterial()  =0;
	
	//! Get the wireframe color
	/*! Access to the wireframe color used by this node.
	\return An IPoint3 (r,g,b) color, with values in the range 0-255
	*/
	virtual IPoint3 & GetWireframeColor() = 0;
	
	//! Check if a Target Node  
	/*!Can be used for exporting targets.  Exporters often treat targets separately to standard nodes
	choosing to export them as part of the node that uses them as targets for example lights and cameras
	\return TRUE if it is a target
	*/
	virtual bool IsTarget()=0;

	//! Check if a Group Head 
	/*! A group in 3ds Max has an owner, a Group Head, this is a dummy node.  The children of a Group Head are the members of the 
	group.  So when you encounter one of these, look for the children as these will be the members of the group.
	\return TRUE if a "Group Head"
	*/
	virtual bool IsGroupOwner()=0;

	//!Check if the node hidden
	/*!This allows access to the hidden property of the node.  A developer may choose to ignore any node that is hidden.  However
	this can be dangerous as many dummy objects get hidden but actually define animation - especially in IK situations
	\return TRUE if hidden
	*/
	virtual bool IsNodeHidden()=0;


};



//!Main scene access
/*! IGameScene is the main entry point for an exporter.  It contains the initialisation routines and access to
the nodes and materials of the scene.  The user can ask IGame to enumerate the whole scene, selected nodes or
single nodes, with or with out hierarchy.
*/

class IGameScene: public MaxHeapOperators
{
	public:
		//! \brief Destructor 
		virtual ~IGameScene() {;}

		//!Set the Property File to use
		/*!You can define the name of the file to use, including full path, that IGame uses to parse for the supported
		parameters.  This means that you could have multiple configs depending on the various games in development.  <b>You must call this
		before you call InitialiseIGame(). </b>  By default the the filename will be the maxroot/plugcfg/IGameProp.XML.
		\param fileName The name of the property file to use inthis session.
		*/
		virtual void SetPropertyFile(const MCHAR * fileName) = 0;

		//! Initialise IGame
		/*! IGame can be initialised specifying whether to search for selected nodes only
		\param selected True if selected are searched for - default false
		\return True if the scene was enumerated - A Possible reason for failure is that the Parameter
		IGameProp.XML file was not found or not parsed correctly
		*/
		virtual bool InitialiseIGame(bool selected = false)=0;

		//!Initialise IGame with a specific node.
		/*!Developer can specify a single node to parse.
		\param root The actual node to parse
		\param Hierarchy lets IGame know to initialise the children of the node
		\return True if the scene was enumerated - A Possible reason for failure is that the Parameter
		IGameProp.XML file was not found or parsed correctly
		*/
		virtual bool InitialiseIGame(INode * root, bool Hierarchy = true)=0;

		//!Initialise IGame with a specific set of nodes.
		/*!Developer can specify a set of nodes to parse.  This could be implemented by creating a Tab using
		the 3ds Max sdk.  Look at Interface::GetSelNodes() and Interface::GetSelNodeCount().
		\param nodes The actual tab containing the nodes to parse
		\param Hierarchy lets IGame know to initialise the children of the node
		\return True if the scene was enumerated - A Possible reason for failure is that the Parameter
		IGameProp.XML file was not found or parsed correctly
		*/
		virtual bool InitialiseIGame(Tab<INode *> nodes, bool Hierarchy = true) = 0;

		//!Get the active filename
		/*!
		\return The current filename
		*/
		virtual  MCHAR * GetSceneFileName()  =0;

		//!Set the static frame
		/*! Specify the frame to use for Time access functions
		\param frameNum The static frame to use - Internally this will be converted to Ticks
		*/
		virtual void SetStaticFrame(int frameNum)=0;

		//!Get the start of the animation range
		/*!This provides the animation start time in ticks
		\return The start time in ticks
		*/
		virtual TimeValue GetSceneStartTime() =0;

		//!Get the end of the animation range
		/*!This provides the animation end time in ticks
		\return The end time in ticks
		*/
		virtual TimeValue GetSceneEndTime() =0;

		//!Get the number of ticks per frame
		/*!
		\return The number of ticks per frame
		*/
		virtual int GetSceneTicks()=0;

		// Get the number of Top level Parent Node
		/*! All node access is provides by parent nodes.  This does not include the Root node as in 3ds Max.  This method
		can be used to traverse the scene
		\return The number of parent nodes
		*/
		virtual int GetTopLevelNodeCount()=0;

		//! Get the complete number of nodes.
		/*! The is useful for implementing a progress bar in the exporter
		\return The total node count
		*/
		virtual int GetTotalNodeCount()=0;

		//! Get the actual toplevel node
		/*!
		A Top level node is a node at the top of the Hierarchy, i.e it has no Parent.
		\param index The index into the the toplevel parent list
		\return A pointer to a IGameNode
		\sa IGameNode
		*/
		virtual IGameNode * GetTopLevelNode(int index)  =0;

		//!  Get the total number of parent materials 
		/*! This does not include sub materials - just the number of actual material containers
		\return The total number of parent materials
		*/
		virtual int GetRootMaterialCount() = 0;

		//! Access to the Root Material
		/*! This is the parent material that is stored on the Node.  The material will host any sub materials and access
		is provided by the IGameMaterial interface.
		\param index The root material to access
		\return A Pointer to an IGameMaterial 
		\sa IGameMaterial
		*/
		virtual  IGameMaterial * GetRootMaterial(int index)  =0;

		//! Access the IGameNode from the supplied INode
		/*! Some IGame methods provide access to INode lists.  You can use this method to retrieve the IGameNode equivelent
		IGame must be initialised first, as the node must be in the Database
		\param node The 3ds Max node to find
		\return A pointer to IGameNode, NULL if not found
		*/
		virtual IGameNode * GetIGameNode(INode * node) =0;

		//! Access the IGameNode from the supplied Node ID
		/*! You can use this method to retrieve the IGameNode based on a Node ID IGame must be initialised first, as the node must be in the Database
		\param NodeID The Node ID to find
		\return A pointer to IGameNode, or NULL if not found
		*/
		virtual IGameNode * GetIGameNode(ULONG NodeID)  =0;

		//! Access the set IGameNodes from the specified IGame Type
		/*! You can use this method to retrieve the IGameNode based on IGameObject::ObjectTypes. IGame must be initialised first, as the node must be in the Database
		\param Type The IGameObject::ObjectTypes to find
		\return A Tab of IGameNodes.  The developer should check the Count() method to see if any were found
		*/
		virtual Tab<IGameNode*> GetIGameNodeByType(IGameObject::ObjectTypes Type) = 0;

		//! Access the IGameMaterial from the supplied Mtl
		/*! You can use this method to retrieve the IGameMaterial equivelent
		IGame must be initialised first, Material doesn't have to be part of database
		\param mat The 3ds Max Material
		\return A pointer to IGameMaterial
		*/
        virtual IGameMaterial * GetIGameMaterial(Mtl * mat) = 0;
		
		//! Access the IGameTextureMap from the supplied Texmap
		/*! You can use this method to retrieve the IGameTextureMap equivelent
		IGame must be initialised first, Texture doesn't have to be part of database
		\param texMap The 3ds Max Texture Map
		\return A pointer to IGameTextureMap
		*/
		virtual IGameTextureMap * GetIGameTextureMap (Texmap * texMap) = 0;

		//! Releases IGame and free all associated memory
		/*! This must be called after you have finished with IGame.  It makes sure all data is freed and general clean up is performed.
		Without calling this there is the risk of corrupting memory, and causing incorrect data to returned the next time IGame is run
		*/
		virtual void ReleaseIGame()=0;
						
};

/*!\fn IGameScene *GetIGameInterface()
\brief A global function to IGameScene, the main starting point in IGame
\return A Pointer to IGameScene
*/
IGAMEEXPORT IGameScene *GetIGameInterface();

/*!\fn float GetIGameVersion()
\brief Provides a way to query for the version of IGame the DLL has been built against
\return A float value with the IGame Version
*/
IGAMEEXPORT float GetIGameVersion();

/*!\fn float GetSupported3DSVersion()
\brief Provides a way to query for the version of 3ds Max that the IGame DLL is compatible with
\return A float value with the 3ds max version.  This currently can either be 4.2, 5.1 or 6.0.  If it is 6.0 or higher then it
will be compiled with the VC7.0 compiler
*/
IGAMEEXPORT float GetSupported3DSVersion();




