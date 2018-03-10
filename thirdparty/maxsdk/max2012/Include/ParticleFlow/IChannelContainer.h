/*! \file IChannelContainer.h
    \brief An interface to manage channels in ParticleContainer.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\ifnpub.h"

// interface ID
#define CHANNELCONTAINER_INTERFACE Interface_ID(0x74f93c00, 0x1eb34600) 

#define GetChannelContainerInterface(obj) ((IChannelContainer*)obj->GetInterface(CHANNELCONTAINER_INTERFACE)) 


class IChannelContainer : public FPMixinInterface
{
public:

	// function IDs
	enum {	
//			kNumChannels,
//			kGetChannel,
			kGetPublicChannel,
			kGetPrivateChannel,
			kAddChannel
	};


	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

//	FN_0(kNumChannels, TYPE_INT, NumChannels);
//	FN_1(kGetChannel, TYPE_IOBJECT, GetChannel, TYPE_INT);
	FN_2(kGetPublicChannel, TYPE_IOBJECT, GetPublicChannel, TYPE_INT, TYPE_INT);
	FN_4(kGetPrivateChannel, TYPE_IOBJECT, GetPrivateChannel, TYPE_INT, TYPE_INT, TYPE_OBJECT, TYPE_INODE);
	FN_1(kAddChannel, TYPE_bool, AddChannel, TYPE_IOBJECT);

	END_FUNCTION_MAP

    /** @defgroup IChannelContainer IChannelContainer.h
    *  @{
    */

    /*! \fn virtual Class_ID GetClassID() = 0;
    *  \brief Returns the unique ID for the particle container class. ID is used for constructing the container when loading since the particle container class is not inherited from class Animatable
    */
	virtual Class_ID GetClassID() = 0;

    /*! \fn virtual int NumChannels() const = 0;
    *  \brief Returns number of channels in the container
    */
	virtual int NumChannels() const = 0;

    /*! \fn virtual IObject*& GetChannel(int i) = 0;
    *  \brief Returns i-th channel in the container
    */
	virtual IObject*& GetChannel(int i) = 0;

    /*! \fn virtual IObject* GetChannel(Interface_ID id, Object* privateOwner=NULL, INode* creatorAction=NULL) = 0;
    *  \brief Returns a channel with the given interface id. If privateOwner is NULL, then the channel is public. If privateOwner is specified, then the channel is private with the given private owner.
    */
	virtual IObject* GetChannel(Interface_ID id, Object* privateOwner=NULL, INode* creatorAction=NULL) = 0;

    // FnPub alternatives for the method above
	/*! \fn IObject* GetPublicChannel(int id_PartA, int id_PartB)
	*  \brief FnPub alternative for the method virtual IObject* GetChannel.
	*  \brief Essentially takes in the two (2) components that make up the full Interface_ID CHANNELCONTAINER_INTERFACE
	*  \param id_PartA: Corresponds to 0x74f93c00.  Look at #define CHANNELCONTAINER_INTERFACE Interface_ID(0x74f93c00, 0x1eb34600)
	*  \param id_partB: Corresponds to 0x1eb34600.  Look at #define CHANNELCONTAINER_INTERFACE Interface_ID(0x74f93c00, 0x1eb34600)
	*/
	IObject* GetPublicChannel(int id_PartA, int id_PartB)
	{ return GetChannel( Interface_ID(id_PartA, id_PartB), NULL); }

	/*!  \fn IObject* GetPrivateChannel(int id_PartA, int id_PartB, Object* privateOwner, INode* creatorAction)
	*  \brief FnPub alternative for method virtual IObject* GetChannel.
	*/
	IObject* GetPrivateChannel(int id_PartA, int id_PartB, Object* privateOwner, INode* creatorAction)
	{ return GetChannel( Interface_ID(id_PartA, id_PartB), privateOwner, creatorAction); }

    /*! \fn virtual bool    AddChannel(IObject* channel) = 0;
    *  \brief Returns true if the channel has been added. The channel may be rejected if there are duplicate interfaces in the added channel and in the container. The ParticleChannelReadWrite and ParticleChannelAmount interfaces don't count since every channel has them.
    */
	virtual bool	AddChannel(IObject* channel) = 0;

    /*! \fn virtual IObject* EnsureChannel(Interface_ID& iChanID, Class_ID& channelClassID, bool isWrapper, Interface_ID& wrappingReadID, Interface_ID& wrappingWriteID, bool isTransferable=TRUE, INode* creatorAction=NULL, Object* privateOwner=NULL, bool* initChannel=NULL) = 0;
    *  \brief Returns a channel object if a channel with the given interface id is in the container. If such channel
       doesn't exist, that container makes an attempt to create and add a channel
       for the interface according to the specifications. The method is not for FnPub intentionally.
      \param iChanID: interface id of channel to be present in the container\n
      \param channelClassID: if interface is not present then create particle channel with the classID\n
      \param isWrapper: if true then the created particle channel is wrapper\n
      \param wrappingReadID & wrappingWriteID: read & write ID for wrapping the particle channel\n
      \param isTransferable: if true then the channel is transferable (contrary to temporary)\n
      \param creatorAction: the parameter is used to record an action that created the channel\n
                      this record can be used later on to identify if the action needs to initialize the
                      channel values for new particles\n
      \param privateOwner: the parameter is used to set a private owner of the channel. Only the action
                      with the same object pointer may access this channel\n
      \param initChannel: if the channel was created by the same action node then it's responsibility of
                      the action to initialize values\n
     */
	virtual IObject* EnsureChannel(Interface_ID& iChanID, Class_ID& channelClassID, 
									bool isWrapper, Interface_ID& wrappingReadID, Interface_ID& wrappingWriteID,
									bool isTransferable=TRUE, INode* creatorAction=NULL, Object* privateOwner=NULL,
									bool* initChannel=NULL) = 0;

    /*! \fn virtual BaseInterface* EnsureInterface(Interface_ID& iChanID, Class_ID& channelClassID, bool isWrapper, Interface_ID& wrappingReadID, Interface_ID& wrappingWriteID, bool isTransferable=TRUE, INode* creatorAction=NULL, Object* privateOwner=NULL, bool* initChannel=NULL) = 0;
    *  \brief Returns an interface if the interface with the given interface id is in the container.
	If such interface doesn't exist, that container makes an attempt to 
	create and add a channel for the interface according to the specifications.
	The method is not for FnPub intentionally.
      \param iChanID: interface id of channel to be present in the container\n
      \param channelClassID: if interface is not present then create particle channel with the classID\n
      \param isWrapper: if true then the created particle channel is wrapper\n
      \param wrappingReadID & wrappingWriteID: read & write ID for wrapping the particle channel\n
      \param isTransferable: if true then the channel is transferable (contrary to temporary)\n
      \param creatorAction: the parameter is used to record an action that created the channel\n
                      this record can be used later on to identify if the action needs to initialize the
                      channel values for new particles\n
      \param privateOwner: the parameter is used to set a private owner of the channel. Only the action
                      with the same object pointer may access this channel\n
      \param initChannel: if the channel was created by the same action node then it's responsibility of
                      the action to initialize values\n
     */
	virtual BaseInterface* EnsureInterface(Interface_ID& iChanID, Class_ID& channelClassID, 
									bool isWrapper, Interface_ID& wrappingReadID, Interface_ID& wrappingWriteID,
									bool isTransferable=TRUE, INode* creatorAction=NULL, Object* privateOwner=NULL,
									bool* initChannel=NULL) = 0;

    /*! \fn virtual BaseInterface* GetPrivateInterface(Interface_ID id, Object* privateOwner) = 0;
    *  \brief Returns an interface of a channel with the given interface id and private owner.
	 \param iChanID: interface id of channel to be present in the container. \n
	 \param privateOwner: the parameter is used to identify a private owner of the channel.\n
     */
	virtual BaseInterface* GetPrivateInterface(Interface_ID id, Object* privateOwner) = 0;

    /*! \fn virtual int RemoveNonTransferableChannels(INode* nextParticleGroup=NULL) = 0;
    *  \brief Returns an interface of a channel with the given interface id and private owner parameters:\n
            iChanID: interface id of channel to be present in the container\n
            privateOwner: the parameter is used to identify a private owner of the channel.
     */
	virtual int RemoveNonTransferableChannels(INode* nextParticleGroup=NULL) = 0;

    /*! \fn virtual IObject* Clone() = 0;
    *  \brief Makes a clone copy of the whole container (all channels), returns new container
     */
	virtual IObject* Clone() = 0;

    /*! \fn virtual bool Copy(IObject* container) = 0;
    *  \brief Makes a copy from the other container, returns true if successful
    */
	virtual bool Copy(IObject* container) = 0;

    /*! \fn virtual IOResult Save(ISave* isave) const = 0;
    *  \brief Saves content of the channel to file
    */
	virtual IOResult Save(ISave* isave) const = 0;

    /*! \fn IOResult Load(ILoad* iload) = 0;
    *  \brief Loads content of the channel from file.
    */
	virtual IOResult Load(ILoad* iload) = 0;

    /*! \fn int MemoryUsed() const = 0;
    *  \brief Returns amount of memory used (in bytes) by the channel container to store the information. This is a total sum of the memory used by all channels and the data from the container itself.
    */
	virtual int MemoryUsed() const = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(CHANNELCONTAINER_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(CHANNELCONTAINER_INTERFACE); }

/*@}*/

};


