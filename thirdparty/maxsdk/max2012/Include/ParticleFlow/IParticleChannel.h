/*! \file IParticleChannel.h
    \brief Channel-generic interfaces IParticleChannel
				This is a part of every particle channel.
				Helps to determine read/write category of the given
				particle interface and to manage memory deallocation
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-09-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
// forward declarations
class IMergeManager;

// interface ID
#define PARTICLECHANNEL_INTERFACE Interface_ID(0x74f93b00, 0x1eb34400)

#define GetParticleChannelInterface(obj) ((IParticleChannel*)obj->GetInterface(PARTICLECHANNEL_INTERFACE))

class IParticleChannel : public FPMixinInterface
{
public:

	// function IDs
	enum {	kClone,
			kIsSimilarChannel,
			kIsTransferable,
			kSetTransferable,
			kIsPrivateChannel,
			kGetPrivateOwner,
			kSetPrivateOwner,
			kGetCreatorAction,
			kSetCreatorAction,
			kGetReadID_PartA,
			kGetReadID_PartB,
			kGetWriteID_PartA,
			kGetWriteID_PartB,
			kSetReadID,
			kSetWriteID
	};

	// chunk IDs for saving and loading
	// developer: you may have your own chunk IDs
	// they are factored here for convenience
	enum {	kChunkCount			= 1000,
			kChunkData			= 1010,
			kChunkGlobalCount	= 1020,
			kChunkSharedCount	= 1021,
			kChunkGlobalValue	= 1030,
			kChunkSharedValue	= 1031,
			kChunkReadID		= 1040,
			kChunkWriteID		= 1050,
			kChunkTransferable	= 1055,
			kChunkPrivate		= 1056,
			kChunkActionHandle	= 1057,
			kChunkValue1		= 1061,
			kChunkValue2		= 1062,
			kChunkValue3		= 1063,
			kChunkValue4		= 1064,
			kChunkValue5		= 1065,
			kChunkValue6		= 1066,
			kChunkValue7		= 1067
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_0(kClone, TYPE_IOBJECT, Clone);
	FN_1(kIsSimilarChannel, TYPE_bool, IsSimilarChannel, TYPE_IOBJECT);
	FN_0(kIsTransferable, TYPE_bool, IsTransferable);
	VFN_1(kSetTransferable, SetTransferable, TYPE_bool);
	FN_0(kIsPrivateChannel, TYPE_bool, IsPrivateChannel);
	FN_0(kGetPrivateOwner, TYPE_OBJECT, GetPrivateOwner);
	VFN_1(kSetPrivateOwner, SetPrivateOwner, TYPE_OBJECT);
	FN_0(kGetCreatorAction, TYPE_INODE, GetCreatorAction);
	VFN_1(kSetCreatorAction, SetCreatorAction, TYPE_INODE);
	FN_0(kGetReadID_PartA, TYPE_DWORD, GetReadID_PartA);
	FN_0(kGetReadID_PartB, TYPE_DWORD, GetReadID_PartB);
	FN_0(kGetWriteID_PartA, TYPE_DWORD, GetWriteID_PartA);
	FN_0(kGetWriteID_PartB, TYPE_DWORD, GetWriteID_PartB);
	VFN_2(kSetReadID, SetReadID, TYPE_DWORD, TYPE_DWORD);
	VFN_2(kSetWriteID, SetWriteID, TYPE_DWORD, TYPE_DWORD);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannel IParticleChannel.h
    *  @{
    */

    	/*! \fn  PFExport IParticleChannel();
    	*  \brief 
      */
	PFExport IParticleChannel();

    	/*! \fn  PFExport IParticleChannel(const Interface_ID& readID, const Interface_ID&
    	*  \brief 
      */
	PFExport IParticleChannel(const Interface_ID& readID, const Interface_ID& writeID);

	/*! \fn virtual Class_ID GetClassID() const = 0;
	*  \brief Returns the unique ID for the channel class. The ID is used for constructing the channel when loading since the channel class is not inherited from class Animatable.
      */
	virtual Class_ID GetClassID() const = 0;

	/*! \fn  virtual IObject*		Clone() const = 0; 
	*  \brief cloning; make a copy of the channel 
      */
	virtual IObject*		Clone() const = 0; 

	/*! \fn virtual IOResult Save(ISave* isave) const = 0;
	*  \brief saving content of the channel to file
      */
	virtual IOResult Save(ISave* isave) const = 0;

	/*! \fn virtual IOResult Load(ILoad* iload) = 0;
	*  \brief loading content of the channel from file
      */
	virtual IOResult Load(ILoad* iload) = 0;

	/*! \fn PFExport bool IsSimilarChannel(IObject* channel) const;
	*  \brief checks if the given IObject is a similar particle channel
      */
	PFExport bool IsSimilarChannel(IObject* channel) const;

	/*! \fn PFExport bool IsTransferable() const;
	*  \brief Get "transferable" status. If particle channel is transferable then it is transferred from one event to another; the data in the channel are glued to particles if particle channel is not transferable then while particles moves to another event, the particle channel is not
      */
	PFExport bool IsTransferable() const;

	/*! \fn PFExport void SetTransferable(bool status);
	*  \brief Set "transferable" status. 
      */
	PFExport void SetTransferable(bool status);

	/*! \fn PFExport bool IsPrivateChannel() const;
	*  \brief A channel can be declared "private" by an action.
	when a channel is declared as a "private" a reference on a "private owner" is given
	then only the "private owner" action will be given access to this channel.
	the technique allows creating several channels with the same data-type (for example,
	position) in the container by different "private owner" actions
	Because of access to particle container, onle an action that created the channel
	can declare itself as a private owner.
      */
	PFExport bool IsPrivateChannel() const;

	/*! \fn PFExport Object* GetPrivateOwner() const;
	*  \brief returns a "private owner" action of the channel
      */
	PFExport Object* GetPrivateOwner() const;

	/*! \fn PFExport void SetPrivateOwner(Object* action);
	*  \brief sets a "private owner" action of the channel
      */
	PFExport void SetPrivateOwner(Object* action);

	/*! \fn PFExport INode* GetCreatorAction() const;
	*  \brief channel data can be viewed by different actions. however an action that created the channel is responsible for data initialization. therefore the action has to keep track
	how (by what action) the channel was created
      */
	PFExport INode* GetCreatorAction() const;

	/*! \fn PFExport void SetCreatorAction(INode* actionNode);
	*  \brief channel data can be viewed by different actions. however an action that created the channel is responsible for data initialization. therefore the action has to keep track
	how (by what action) the channel was created
      */
	PFExport void SetCreatorAction(INode* actionNode);

	/*! \fn PFExport DWORD GetReadID_PartA() const;
	*  \brief Set/get a concrete channel ID for a generic data particle channel. Access read-interface ID of the wrapping channel.
      */
	PFExport DWORD GetReadID_PartA() const;

	/*! \fn PFExport DWORD GetReadID_PartB() const;
	*  \brief Set/get a concrete channel ID for a generic data particle channel. Access read-interface ID of the wrapping channel.
      */
	PFExport DWORD GetReadID_PartB() const;

	/*! \fn PFExport DWORD GetWriteID_PartA() const;
	*  \brief access write-interface ID of the wrapping channel
      */
	PFExport DWORD GetWriteID_PartA() const;

	/*! \fn PFExport DWORD GetWriteID_PartB() const;
	*  \brief access write-interface ID of the wrapping channel
      */
	PFExport DWORD GetWriteID_PartB() const;

	/*! \fn PFExport void SetReadID(DWORD interfaceID_PartA, DWORD interfaceID_PartB);
	*  \brief set up read-interface ID of the wrapping channel
      */
	PFExport void SetReadID(DWORD interfaceID_PartA, DWORD interfaceID_PartB);

	/*! \fn PFExport void SetWriteID(DWORD interfaceID_PartA, DWORD interfaceID_PartB);
	*  \brief set up write-interface ID of the wrapping channel
      */
	PFExport void SetWriteID(DWORD interfaceID_PartA, DWORD interfaceID_PartB);

	/*! \fn PFExport const Interface_ID& GetReadID() const;
	*  \brief methods that are not in FnPub interface. Access read-interface ID of the wrapping channel.
      */ 	
	PFExport const Interface_ID& GetReadID() const;

	/*! \fn PFExport const Interface_ID& GetWriteID() const;
	*  \brief access write-interface ID of the wrapping channel
      */
	PFExport const Interface_ID& GetWriteID() const;

	/*! \fn PFExport void SetReadID(const Interface_ID& id);
	*  \brief set up read-interface ID of the wrapping channel
      */
	PFExport void SetReadID(const Interface_ID& id);

	/*! \fn PFExport void SetWriteID(const Interface_ID& id);
	*  \brief set up write-interface ID of the wrapping channel
      */
	PFExport void SetWriteID(const Interface_ID& id);

	/*! \fn PFExport void CloneChannelCore(IParticleChannel* getFrom);
	*  \brief when cloning a channel, data about transferable, privateOwner, creatorAction, readID & write ID should be cloned. the following method does that: it clones the core data from the given channel
      */
	PFExport void CloneChannelCore(IParticleChannel* getFrom);

	/*! \fn PFExport void UpdateCreatorHandle(IMergeManager* pMM);
	*  \brief when cached channel data are merged the stored action creator handle could be invalid.
		the method lets the channel to update the action handle to the proper one
      */
	PFExport void UpdateCreatorHandle(IMergeManager* pMM);

    	/*! \fn  virtual int MemoryUsed() const = 0;
    	*  \brief returns amount of memory used (in bytes) by the channel to store the information
      */
	virtual int MemoryUsed() const = 0;

    	/*! \fn  FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNEL_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNEL_INTERFACE); }

protected:
	bool m_isTransferable;
	bool m_isPrivate;
	mutable Object* m_privateOwner; // could be derived from creatorAction
	mutable INode* m_creatorAction; // could be derived from creatorHandle
	ULONG m_creatorHandle;
	// wrapper ids
	Interface_ID m_readID, m_writeID;

	int		numRefs()	const	{ return m_numRefs; }
	int&	_numRefs()			{ return m_numRefs; }
private:
	int m_numRefs;
};




