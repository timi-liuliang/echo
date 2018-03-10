/*****************************************************************************

	FILE: IContainerManager.h

	DESCRIPTION: Describes the interface for global container functionality.

	CREATED BY:	Nicolas Leonard

	HISTORY:	July 9th, 2008	Creation

 	Copyright (c) 2008, All Rights Reserved.
 *****************************************************************************/
#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"
#include "AssetManagement/AssetUser.h"
#include "IContainerObject.h"

// forward declarations
class INodeTab;


//! \brief The interface for global container functionality.
/*! Operations on a specific container can be accessed through the container itself. 
\see IContainerObject
*/
class IContainerManager : public FPStaticInterface
{
public:
	//! \brief Create a container
	/*! \return the container node if successfully created */
	/*! \remarks If created, the container node will be selected */
	/*! \remarks The container node will be created at the average of the content nodes, or at the origin if the Tab is empty */
	/*! \param[in] contentNodes - Nodes to be added as content of the container */
	virtual INode* CreateContainer(INodeTab& contentNodes) = 0;

	//! \brief Inherit a container into the scene
	/*! \return the container node if a container was successfully inherited */
	/*! \param[in] asset - The container asset representing the definition file to be used */
	/*! \remarks If the asset is invalid, the user will be presented with a file dialog. */
	virtual INode* CreateInheritedContainer(const MaxSDK::AssetManagement::AssetUser& asset) = 0;

	//! \brief Test if a node is in a container or not
	/*! \param[in] node - The node that we are testing to see if it is in a container */
	/*! \return the container interface if the node is in a container */
	virtual IContainerObject* IsInContainer(INode *node) = 0;

	//! \brief Test if a node is a container or not
	/*! \param[in] node - The node that we are testing to see if it is a container */
	/*! \return the container interface if the node is a container */
	virtual IContainerObject* IsContainerNode(INode *node) = 0;

	// Function IDs for function publishing
	enum 
	{ 
		IMPORT_CONTAINER_ID,
		CREATE_CONTAINER_ID,
		IS_IN_CONTAINER_ID,
		IS_CONTAINER_NODE_ID,
	};
};

//! \brief IContainerManager interface ID
#define IID_IContainerManager Interface_ID(0xD951AEE9, 0x5769E48B)

//! \brief Returns the container manager interface 
inline IContainerManager* GetContainerManagerInterface() { 
	return static_cast<IContainerManager*>(GetCOREInterface(IID_IContainerManager)); 
}


//==============================================================================
// class IContainerPreferences
//
//! \brief This interface provides access to the container preferences.
//==============================================================================
class IContainerPreferences: public FPStaticInterface
{
public:
	//! \brief Possible values for global container status display.  
	enum DisplayStatusOverride { 
		//! \brief Always display status, regardless of individual container setting
		eAlways = 0x0, 
		//! \brief Never display status, regardless of individual container setting
		eNever, 
		//! \brief Display status according to individual container setting
		ePerContainer
	};

	//! \brief Returns the access type that will be used when a container is saved in 3ds max 2010 format
	virtual IContainerObject13::AccessType SaveAsPreviousAccessType() const = 0;
	//! \brief Sets the access type that will be used when a container is saved in 3ds max 2010 format 
	virtual void SetSaveAsPreviousAccessType(IContainerObject13::AccessType accessType) = 0;

	//! \brief Returns the global override of individual container viewport status display.  \see DisplayStatusOverride
	virtual DisplayStatusOverride DisplayStatus() const = 0;
	//! \brief Sets a potential override of individual container viewport status display.  \see DisplayStatusOverride
	virtual void SetDisplayStatus(DisplayStatusOverride displayStatus) = 0;

	//! \brief Returns whether automatic detection of out of date containers is set
	virtual bool NoUpdateCheck() const = 0;
	//! \brief Sets automatic detection of out of date containers
	virtual void SetNoUpdateCheck(bool noUpdateCheck) = 0;

	//! \brief Returns whether an eventual out of date source definition will be updated on load
	virtual bool UpdateOnLoad() const = 0;
	//! \brief Sets whether an eventual out of date source definition will be updated on load
	virtual void SetUpdateOnLoad(bool updateOnLoad) = 0;

	//! \brief Returns whether an eventual out of date source definition will be updated on reload
	virtual bool UpdateOnReload() const = 0;
	//! \brief Sets whether an eventual out of date source definition will be updated on reload
	virtual void SetUpdateOnReload(bool updateOnReload) = 0;

	//! \brief Returns the time interval between container status updates, in seconds
	virtual float StatusUpdateInterval() const = 0;
	//! \brief Sets the time interval between container status updates, in seconds
	virtual void SetStatusUpdateInterval(float statusInterval) = 0;
};

//! \brief IContainerPreferences interface ID
#define IID_IContainerPreferences Interface_ID(0xFA3B4E00, 0xEF1BA027)

//! \brief Returns the container preferences interface 
inline IContainerPreferences* GetContainerPreferences() {
	return static_cast<IContainerPreferences*>(GetCOREInterface(IID_IContainerPreferences));
}
