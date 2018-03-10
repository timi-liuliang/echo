/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PXS_ISLAND_MANAGER_AUX_H
#define PXS_ISLAND_MANAGER_AUX_H

#include "CmBitMap.h"

namespace physx
{

class PxsRigidBody;
class PxsArticulation;
class PxsContactManager;
struct PxsConstraint;

namespace Cm
{
	class EventProfiler;
}

#if PX_USE_16_BIT_HANDLES
typedef PxU16 NodeType;
typedef PxU16 EdgeType;
typedef PxU16 IslandType;
#define INVALID_NODE 0xffff
#define INVALID_EDGE 0xffff
#define INVALID_ISLAND 0xffff
#define INVALID_EDGE_REPORT "Exceeded 65535 island manager edges - all subsequent rigid body contact and constraint pairs will be neglected"
#else
typedef PxU32 NodeType;
typedef PxU32 EdgeType;
typedef PxU32 IslandType;
#define INVALID_NODE 0xffffffff
#define INVALID_EDGE 0xffffffff
#define INVALID_ISLAND 0xffffffff
#define INVALID_EDGE_REPORT "Exceeded 4294967295 island manager edges - all subsequent rigid body contact and constraint pairs will be neglected"
#endif

typedef size_t PxsArticulationLinkHandle;

typedef PxsArticulationLinkHandle PxsNodeType;

struct PxsIndexedInteraction
{
	enum IndexType
	{
		eBODY = 0,
		eKINEMATIC = 1,
		eARTICULATION = 2,
		eWORLD = 3
	};

	union
	{
		PxsNodeType					solverBody0;
		PxsArticulationLinkHandle	articulation0;
	};

	union
	{
		PxsNodeType					solverBody1;
		PxsArticulationLinkHandle	articulation1;
	};

	PxU8 indexType0;
	PxU8 indexType1;
	PxU8 pad[2];
};

struct PxsIndexedContactManager : public PxsIndexedInteraction
{
	PxsContactManager* contactManager;
	PxsIndexedContactManager(PxsContactManager* cm) : contactManager(cm) {}
};

struct PxsIndexedConstraint : public PxsIndexedInteraction
{
	PxsConstraint* constraint;
	PxsIndexedConstraint(PxsConstraint* c) : constraint(c) {}
};
#if !defined(PX_P64)
PX_COMPILE_TIME_ASSERT(0==(sizeof(PxsIndexedConstraint) & 0x0f));
#endif

// There are always islandCount+1 sets of islandIndices
struct PxsIslandObjects
{
	PxsRigidBody*const*			bodies;	
	PxsArticulation*const*		articulations;
	void*const*					articulationOwners;
	PxsIndexedContactManager*	contactManagers;
	PxsIndexedConstraint*		constraints;

	PxsIslandObjects() : bodies(NULL), articulations(NULL), articulationOwners(NULL), contactManagers(NULL), constraints(NULL)
	{
	}
};

class PxsIslandIndices
{
public:

	PxsIslandIndices(){}
	~PxsIslandIndices(){}

	PX_FORCE_INLINE void setHasStaticContact(const bool b) 
	{
		hasStaticContact = NodeType(b ? 1 : 0);
	}
	PX_FORCE_INLINE bool getHasStaticContact() const
	{
		return (1 & hasStaticContact) ? true : false;
	}
	NodeType	bodies;
	NodeType	articulations : 8*sizeof(NodeType)-1;

private:
	NodeType	hasStaticContact : 1;

public:
	EdgeType	contactManagers;
	union
	{
		EdgeType	constraints;	// standard usecase if PxsIslandIndices is used to describe an island for the solver
		IslandType	islandId;		// used for tracking islands that need a second pass
	};
};
PX_COMPILE_TIME_ASSERT(0==(0x07 & sizeof(PxsIslandIndices)));


template <class T, T INVLD> class PxsIslandManagerHook
{
	friend class PxsIslandManager;
	T index;

public:

	static const T INVALID = INVLD;

	PX_FORCE_INLINE PxsIslandManagerHook(): index(INVLD) {}
	PX_FORCE_INLINE PxsIslandManagerHook(const T id): index(id) {}
	PX_FORCE_INLINE PxsIslandManagerHook(const PxsIslandManagerHook<T,INVLD>& src) : index(src.index) {}
	PX_FORCE_INLINE ~PxsIslandManagerHook(){}

	PX_FORCE_INLINE bool isManaged() const { return index!=INVLD; }

private:
};

typedef PxsIslandManagerHook<NodeType,INVALID_NODE> PxsIslandManagerNodeHook;
typedef PxsIslandManagerHook<EdgeType,INVALID_EDGE> PxsIslandManagerEdgeHook;
typedef PxsIslandManagerHook<IslandType,INVALID_ISLAND> PxsIslandManagerIslandHook;

struct NarrowPhaseContactManager
{
	PxsContactManager* mCM;
	EdgeType mEdgeId;
};

class Node 
{
public:

	enum
	{
		INVALID = INVALID_NODE
	};

	enum
	{
		//Types of node
		eKINEMATIC				= (1<<0),
		eARTICULATED			= (1<<1),
		eARTICULATEDROOT		= (1<<2),
		//States of node
		eNOTREADYFORSLEEPING	= (1<<3),
		eINSLEEPINGISLAND		= (1<<4), // note: for kinematics it is the job of the external system to set this flag appropriately
		eDELETED				= (1<<5),
		eNEW					= (1<<6)
	};

	PX_FORCE_INLINE Node()
		: mRigidBodyOwner(NULL),
		  mIslandId(INVALID_ISLAND),
		  mFlags(0)
	{
	}

	PX_FORCE_INLINE void init()
	{
		release();
	}

	PX_FORCE_INLINE void release()
	{
		mRigidBodyOwner=NULL;
		mIslandId=INVALID_ISLAND;
		mFlags=0;
	}

	PX_FORCE_INLINE void setKinematic(const bool isKinematic)
	{
		if(isKinematic) 
		{
			mFlags |= eKINEMATIC;
		}
		else
		{
			mFlags &= ~eKINEMATIC;
		}
	}

	PX_FORCE_INLINE bool getIsKinematic() const
	{
		return !!(mFlags & eKINEMATIC);
	}

	PX_FORCE_INLINE void setNotReadyForSleeping()
	{
		mFlags |= eNOTREADYFORSLEEPING;
	}

	PX_FORCE_INLINE void setAwakeAndNotReadyForSleeping()
	{
		mFlags = PxU8((mFlags | eNOTREADYFORSLEEPING) & ~eINSLEEPINGISLAND);
	}

	PX_FORCE_INLINE void setAsleepAndReadyForSleeping()
	{
		mFlags = PxU8((mFlags | eINSLEEPINGISLAND) & ~eNOTREADYFORSLEEPING);
	}

	PX_FORCE_INLINE void setReadyForSleeping()
	{
		mFlags &= ~eNOTREADYFORSLEEPING;
	}

	PX_FORCE_INLINE bool getIsReadyForSleeping() const
	{
		return (!(mFlags & Node::eNOTREADYFORSLEEPING));
	}

	PX_FORCE_INLINE void setIsInSleepingIsland() 
	{
		mFlags |= Node::eINSLEEPINGISLAND;
	}

	PX_FORCE_INLINE void clearIsInSleepingIsland()
	{
		mFlags &= ~Node::eINSLEEPINGISLAND;
	}

	PX_FORCE_INLINE bool getIsInSleepingIsland() const
	{
		return !!(mFlags & Node::eINSLEEPINGISLAND);
	}

	PX_FORCE_INLINE void setIslandId(const IslandType islandId)
	{
		mIslandId=islandId;
	}

	PX_FORCE_INLINE IslandType getIslandId() const
	{
		return mIslandId;
	}

	PX_FORCE_INLINE void setRigidBodyOwner(void* bodyOwner)
	{
		PX_ASSERT(!getIsArticulated());
		mRigidBodyOwner=bodyOwner;
	}

	PX_FORCE_INLINE void* getRigidBodyOwner() 
	{
		PX_ASSERT(!getIsArticulated());
		return mRigidBodyOwner;
	}

	PX_FORCE_INLINE PxsRigidBody* getRigidBody(const PxU32 rigidBodyOffset) const
	{
		PX_ASSERT(!getIsArticulated());
		return (PxsRigidBody*)((PxU8*)mRigidBodyOwner + rigidBodyOffset);
	}

	PX_FORCE_INLINE void setArticulated()
	{
		mArticulationLink=INVALID_NODE;
		mFlags |= eARTICULATED;
	}

	PX_FORCE_INLINE bool getIsArticulated() const
	{
		return !!(mFlags & eARTICULATED);
	}

	PX_FORCE_INLINE void setArticulationLink(const PxsArticulationLinkHandle articulationLink)
	{
		PX_ASSERT(getIsArticulated());
		mArticulationLink=articulationLink;
	}

	PX_FORCE_INLINE PxsArticulationLinkHandle getArticulationLink() const 
	{
		PX_ASSERT(getIsArticulated());
		PX_ASSERT(!getIsRootArticulationLink());
		return mArticulationLink;
	}

	PX_FORCE_INLINE void setRootArticulationId(const size_t rootArticulationId)
	{
		PX_ASSERT(getIsArticulated());
		mFlags |= eARTICULATEDROOT;
		mRootArticulationId=rootArticulationId;
	}

	PX_FORCE_INLINE bool getIsRootArticulationLink() const 
	{
		PX_ASSERT(getIsArticulated());
		return !!(mFlags & eARTICULATEDROOT);
	}

	PX_FORCE_INLINE size_t getRootArticulationId() const 
	{
		PX_ASSERT(getIsArticulated());
		PX_ASSERT(getIsRootArticulationLink());
		return mRootArticulationId;
	}

	PX_FORCE_INLINE void setIsDeleted()
	{
		mFlags |= eDELETED;
	}

	PX_FORCE_INLINE bool getIsDeleted() const
	{
		return !!(mFlags & eDELETED);
	}

	PX_FORCE_INLINE void clearIsDeleted()
	{
		mFlags &= ~eDELETED;
	}

	PX_FORCE_INLINE void setIsNew() 
	{
		mFlags |= eNEW;
	}

	PX_FORCE_INLINE bool getIsNew() const
	{
		return !!(mFlags & eNEW);
	}

	PX_FORCE_INLINE void clearIsNew()
	{
		mFlags &= ~eNEW;
	}

	PX_FORCE_INLINE PxU16 getFlags() const 
	{
		return mFlags;
	}

	PX_FORCE_INLINE void setFlags(const PxU8 flagsIn)
	{
		mFlags=flagsIn;
	}

private:

	union
	{
		void*						mRigidBodyOwner;
		PxsArticulationLinkHandle	mArticulationLink;
		size_t						mRootArticulationId;
	};
	IslandType mIslandId;
	PxU8 mFlags;
};

class Edge
{
public:

	enum
	{
		INVALID = INVALID_EDGE
	};

	enum
	{
		//EDGE_TYPE_RIGIDCM =		0,
		EDGE_TYPE_CONSTRAINT_OR_ARTICULATION =	(1<<0),
		EDGE_STATE_CONNECTED =					(1<<1),
		EDGE_STATE_CREATED =					(1<<2),
		EDGE_STATE_REMOVED =					(1<<3),
		EDGE_ALL_FLAGS =						(EDGE_TYPE_CONSTRAINT_OR_ARTICULATION | EDGE_STATE_CONNECTED | EDGE_STATE_CREATED | EDGE_STATE_REMOVED)
	};

	Edge()
		: mNode1(INVALID_NODE),
 		  mNode2(INVALID_NODE),
		  mContactManager(NULL)
	{
	}

	Edge(NodeType node1, NodeType node2)
		: mNode1(node1),
		  mNode2(node2),
		  mContactManager(NULL)
	{
	}

	PX_FORCE_INLINE void init()
	{
		release();
	}

	PX_FORCE_INLINE void release()
	{
		mNode1=INVALID_NODE;
		mNode2=INVALID_NODE;
		mContactManager=NULL;
	}

	PX_FORCE_INLINE bool getIsTypeCM() const 
	{
		return (0==((size_t)mContactManager & (EDGE_TYPE_CONSTRAINT_OR_ARTICULATION)));
	}
	PX_FORCE_INLINE bool getIsTypeConstraint() const 
	{
		return ( ((size_t)mContactManager & EDGE_TYPE_CONSTRAINT_OR_ARTICULATION) &&  (0 != ((size_t)mContactManager & ~EDGE_ALL_FLAGS)) );
	}
	PX_FORCE_INLINE bool getIsTypeArticulation() const 
	{
		return ( ((size_t)mContactManager & EDGE_TYPE_CONSTRAINT_OR_ARTICULATION) &&  (0 == ((size_t)mContactManager & ~EDGE_ALL_FLAGS)) );
	}

	PX_FORCE_INLINE void setNode1(const NodeType node1) {mNode1=node1;}
	PX_FORCE_INLINE void setNode2(const NodeType node2) {mNode2=node2;}
	PX_FORCE_INLINE NodeType getNode1() const {return mNode1;}
	PX_FORCE_INLINE NodeType getNode2() const {return mNode2;}

	PX_FORCE_INLINE void setCM(PxsContactManager* cm)  
	{
		PX_ASSERT(0 == ((size_t)cm & 0x0f));
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(NULL==getCM());
		mContactManager = (PxsContactManager*)((size_t)cm | (0 | (size_t)mContactManager));
	}
	PX_FORCE_INLINE void clearCM()
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(getCM());
		PX_ASSERT(getIsTypeCM());
		PX_ASSERT(!getIsTypeConstraint());
		PX_ASSERT(!getIsTypeArticulation());
		mContactManager = (PxsContactManager*)((size_t)mContactManager & 0x0f);
	}
	PX_FORCE_INLINE PxsContactManager* getCM()  
	{
		PX_ASSERT(getIsTypeCM());
		PX_ASSERT(!getIsTypeConstraint());
		PX_ASSERT(!getIsTypeArticulation());
		return (PxsContactManager*)((size_t)mContactManager & ~0x0f);
	}

	PX_FORCE_INLINE void setConstraint(PxsConstraint* constraint)
	{
		PX_ASSERT(0 == ((size_t)constraint & 0x0f));
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(NULL==getConstraint());
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeCM());
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeArticulation());
		mConstraint = (PxsConstraint*)((size_t)constraint | (EDGE_TYPE_CONSTRAINT_OR_ARTICULATION | (size_t)mConstraint));
	}
	PX_FORCE_INLINE PxsConstraint* getConstraint()
	{
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeCM());
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeArticulation());
		return (PxsConstraint*)((size_t)mConstraint & ~0x0f);
	}

	PX_FORCE_INLINE void setArticulation()
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(NULL==getConstraint());
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeCM());
		PX_ASSERT((0==((size_t)mConstraint & ~EDGE_ALL_FLAGS)) || !getIsTypeConstraint());
		mConstraint = (PxsConstraint*)(0 | (EDGE_TYPE_CONSTRAINT_OR_ARTICULATION | (size_t)mConstraint));
	}

	PX_FORCE_INLINE void setConnected() 
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(!getIsConnected());
		mConstraint = (PxsConstraint*)((size_t)mConstraint | EDGE_STATE_CONNECTED);
	}
	PX_FORCE_INLINE void setUnconnected() 
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(getIsConnected());
		mConstraint = (PxsConstraint*)((size_t)mConstraint & ~EDGE_STATE_CONNECTED);
	}
	PX_FORCE_INLINE bool getIsConnected() const
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		return !!((size_t)mConstraint & EDGE_STATE_CONNECTED);
	}

	PX_FORCE_INLINE void setRemoved() 
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(!getIsTypeCM() || !getCM());
		PX_ASSERT(!getIsRemoved());
		mConstraint = (PxsConstraint*)((size_t)mConstraint | EDGE_STATE_REMOVED);
	}
	PX_FORCE_INLINE bool getIsRemoved() const
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		return !!((size_t)mConstraint & EDGE_STATE_REMOVED);
	}

	PX_FORCE_INLINE void setCreated()
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		PX_ASSERT(!getIsTypeCM() || !getCM());
		PX_ASSERT(!getIsCreated());
		mConstraint = (PxsConstraint*)((size_t)mConstraint | EDGE_STATE_CREATED);
	}
	PX_FORCE_INLINE bool getIsCreated() const
	{
		PX_ASSERT(mNode1!=INVALID_NODE || mNode2!=INVALID_NODE);
		return !!((size_t)mConstraint & EDGE_STATE_CREATED);
	}
	PX_FORCE_INLINE void clearCreated()
	{
		mConstraint = (PxsConstraint*)((size_t)mConstraint & ~EDGE_STATE_CREATED);
	}

private:

	NodeType mNode1;
	NodeType mNode2;
	union
	{
		PxsConstraint*	mConstraint;
		PxsContactManager* mContactManager;
	};
};

class Island
{
public:

	enum
	{
		INVALID = INVALID_ISLAND
	};

	Island()
		: mStartNodeId(INVALID_NODE),
		mStartEdgeId(INVALID_EDGE),
		mEndNodeId(INVALID_NODE),
		mEndEdgeId(INVALID_EDGE)
	{
	}

	PX_FORCE_INLINE void init()
	{
		release();
	}

	PX_FORCE_INLINE void release()
	{
		mStartEdgeId=INVALID_EDGE;
		mStartNodeId=INVALID_NODE;
		mEndEdgeId=INVALID_EDGE;
		mEndNodeId=INVALID_NODE;
	}

	NodeType mStartNodeId;
	EdgeType mStartEdgeId;
	NodeType mEndNodeId;
	EdgeType mEndEdgeId;

};

class ArticulationRoot
{
public:

	enum
	{
		INVALID = INVALID_NODE
	};

	ArticulationRoot()
		: mArticulationLinkHandle(INVALID_NODE),
		  mArticulationOwner(NULL)
	{
	}

	PX_FORCE_INLINE void init()
	{
		release();
	}

	PX_FORCE_INLINE void release()
	{
		mArticulationLinkHandle=INVALID_NODE;
		mArticulationOwner=NULL;
	}

public:

	PxsArticulationLinkHandle mArticulationLinkHandle;
	void* mArticulationOwner;
};

template <class T, class IdType> class ElemManager
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

#ifndef __SPU__

	ElemManager()
		: mElems(NULL),
		  mFreeElems(NULL),
		  mCapacity(0),
		  mNextFreeElem(PxU32(T::INVALID)),
		  mNumFreeElems(0)
	{
	}

	virtual ~ElemManager()
	{
	}

	void init(const PxU32 capacity)
	{
		if(capacity > mCapacity)
			resize(capacity);
	}

	PX_FORCE_INLINE IdType getAvailableElem()
	{
		PX_ASSERT(mCapacity>0);
		if(T::INVALID==mNextFreeElem)
		{
			PX_ASSERT(0 == mNumFreeElems);
			bool success = grow();
			if(!success)
			{
				return (IdType)T::INVALID;
			}
		}

		const IdType freeElem=(IdType)mNextFreeElem;
		mNextFreeElem=mFreeElems[freeElem];
		mFreeElems[freeElem]=IdType(T::INVALID);
		mElems[freeElem].init();
		mNumFreeElems--;
		return freeElem;
	}

#else

	void set(T* allElems, IdType* freeElems, const IdType nextFreeElem, const PxU32 numFreeElems, const PxU32 capacity)
	{
		mElems=allElems;
		mFreeElems=freeElems;
		mNextFreeElem=nextFreeElem;
		mNumFreeElems=numFreeElems;
		mCapacity=capacity;
	}

#endif 

	PX_FORCE_INLINE IdType getAvailableElemNoResize()
	{
		PX_ASSERT(mCapacity>0);
		PX_ASSERT(mNumFreeElems>0);
		PX_ASSERT(INVALID_ISLAND!=mNextFreeElem);
		const IdType freeElem=(IdType)mNextFreeElem;
		mNextFreeElem=mFreeElems[freeElem];
		mFreeElems[freeElem]=IdType(T::INVALID);
		mElems[freeElem].init();
		mNumFreeElems--;
		return freeElem;
	}

	PX_FORCE_INLINE void release(const IdType id)
	{
		PX_ASSERT(id < mCapacity);
		T& t=mElems[id];
		t.release();
		mFreeElems[id]=(IdType)mNextFreeElem;
		mNextFreeElem=id;
		mNumFreeElems++;
	}

	PX_FORCE_INLINE const T& get(const IdType id) const
	{
		PX_ASSERT(id < mCapacity);
		return mElems[id];
	}

	PX_FORCE_INLINE  T& get(const IdType id)
	{
		PX_ASSERT(id < mCapacity);
		return mElems[id];
	}

	PX_FORCE_INLINE  T* getAll()
	{
		return mElems;
	}

	PX_FORCE_INLINE const T* getAll() const
	{
		return mElems;
	}

	PX_FORCE_INLINE PxU32 getCapacity() const 
	{
		return mCapacity;
	}

#ifdef __SPU__
	PX_FORCE_INLINE IslandType getNextFreeElemId() const
	{
		return mNextFreeElem;
	}
#else
	bool grow()
	{
		if((mCapacity-1) == T::INVALID)
		{
			return false;
		}

		PX_ASSERT(0 == mNumFreeElems);
		resize(mCapacity*2);
		return true;
	}
	virtual void resize(const PxU32 newCapacity) = 0;
#endif

#ifdef PX_DEBUG
	PX_FORCE_INLINE PxU32 computeNumAvailableElems() const
	{
		PxU32 count=0;
		PxU32 nextFree=mNextFreeElem;
		while(T::INVALID!=nextFree)
		{
			count++;
			nextFree=mFreeElems[nextFree];
		}
		return count;
	}
#endif

	PX_FORCE_INLINE PxU32 getNumAvailableElems() const 
	{
		PX_ASSERT(mNumFreeElems == computeNumAvailableElems());
		return mNumFreeElems;
	}

protected:

	T* mElems;
	IdType* mFreeElems;
	PxU32 mCapacity;
	PxU32 mNextFreeElem;
	PxU32 mNumFreeElems;

	void setupNewFreeElems(const PxU32 oldCapacity, const PxU32 newCapacity)
	{
		mFreeElems[newCapacity-1] = (IdType)mNextFreeElem;
		for(PxU32 i = oldCapacity; i < (newCapacity-1); i++)
		{
			mFreeElems[i] = (IdType)(i + 1);
		}
		mNextFreeElem = oldCapacity;
		mNumFreeElems += ((newCapacity - oldCapacity) + ((newCapacity == (T::INVALID + 1)) ? -1 : 0));
	}
};

class NodeManager : public ElemManager<Node,NodeType>
{
public:

	friend class PxsIslandManager;

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

	NodeManager() 
		: ElemManager<Node,NodeType>(),
		  mNextNodeIds(NULL)
	{
		for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
		{
			mBitmapWords[i] = NULL;
			mBitmapWordCounts[i] = 0;
			mBitmaps[i] = (Cm::BitMap*)mBitmapBuffers[i];
		}
	}

#ifdef __SPU__
	void set(Node* allNodes, NodeType* freeNodes, const NodeType nextFreeElem, const PxU32 numFreeElems, const PxU32 capacity, NodeType* nextNodeIds, PxU32** nodeBitmapWords, PxU32* nodeBitmapWordCounts)
	{
		ElemManager<Node,NodeType>::set(allNodes,freeNodes,nextFreeElem,numFreeElems,capacity);
		mNextNodeIds=nextNodeIds;
		for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
		{
			mBitmaps[i] = (Cm::BitMap*)mBitmapBuffers[i];
			mBitmapWords[i] = nodeBitmapWords[i];
			mBitmapWordCounts[i] = nodeBitmapWordCounts[i];
			mBitmaps[i]->setWords(nodeBitmapWords[i], nodeBitmapWordCounts[i]);
		}
	}
#endif

#ifndef __SPU__
	virtual ~NodeManager()
	{
		PX_FREE(mElems);
	}
#endif

	PX_FORCE_INLINE NodeType* getNextNodeIds() {return mNextNodeIds;}
	PX_FORCE_INLINE const NodeType* getNextNodeIds() const {return mNextNodeIds;}

	PX_FORCE_INLINE void setKinematicNode(const NodeType id)
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(!mBitmaps[eKINEMATIC]->test(id));
		PX_ASSERT(get((NodeType)id).getIsKinematic());
		mBitmaps[eKINEMATIC]->set(id);
	}
	PX_FORCE_INLINE void clearKinematicNode(const NodeType id)
	{
		PX_ASSERT(id<mCapacity);
		PX_ASSERT(mBitmaps[eKINEMATIC]->test(id));
		PX_ASSERT(!get((NodeType)id).getIsKinematic());
		mBitmaps[eKINEMATIC]->reset(id);
	}

	PX_FORCE_INLINE void setNotReadyForSleeping(const NodeType id)
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(!mBitmaps[eNOT_READY_FOR_SLEEPING]->test(id));
		PX_ASSERT(!get((NodeType)id).getIsReadyForSleeping());
		mBitmaps[eNOT_READY_FOR_SLEEPING]->set(id);
	}
	PX_FORCE_INLINE void clearNotReadyForSleeping(const NodeType id)
	{
		PX_ASSERT(id < mCapacity);
		PX_ASSERT(mBitmaps[eNOT_READY_FOR_SLEEPING]->test(id));
		PX_ASSERT(get((NodeType)id).getIsReadyForSleeping());
		mBitmaps[eNOT_READY_FOR_SLEEPING]->reset(id);
	}

	PX_FORCE_INLINE void setKinematicStateChange(const NodeType id)
	{
		PX_ASSERT(id<mCapacity);

		if(get(id).getIsNew())
			return;

		if(!mBitmaps[eKINEMATIC_CHANGE]->test(id))
		{
			mBitmaps[eKINEMATIC_CHANGE]->set(id);
		}
		else
		{
			mBitmaps[eKINEMATIC_CHANGE]->reset(id);
		}
	}
	PX_FORCE_INLINE void clearKinematicStateChanges()
	{
		mBitmaps[eKINEMATIC_CHANGE]->clearFast();
	}


	PX_FORCE_INLINE void setNotReadyForSleepStateChange(const NodeType id)
	{
		PX_ASSERT(id<mCapacity);
		if(get(id).getIsNew())
			return;

		if(!mBitmaps[eNOT_READY_FOR_SLEEPING_CHANGE]->test(id))
		{
			mBitmaps[eNOT_READY_FOR_SLEEPING_CHANGE]->set(id);
		}
		else
		{
			mBitmaps[eNOT_READY_FOR_SLEEPING_CHANGE]->reset(id);
		}
	}
	PX_FORCE_INLINE void clearNotReadyForSleepStateChanges()
	{
		mBitmaps[eNOT_READY_FOR_SLEEPING_CHANGE]->clearFast();
	}

	PX_FORCE_INLINE void deleteNode(const NodeType nodeId)
	{
		for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
		{
			mBitmaps[i]->reset(nodeId);
		}
	}

	PX_FORCE_INLINE const Cm::BitMap& getBitmap(const PxU32 type) const
	{
		PX_ASSERT(type < eMAX_NB_BITMAPS);
		return *mBitmaps[type];
	}

	enum 
	{
		eKINEMATIC,
		eKINEMATIC_CHANGE,
		eNOT_READY_FOR_SLEEPING,
		eNOT_READY_FOR_SLEEPING_CHANGE,
		eMAX_NB_BITMAPS
	};

private:

	NodeType* mNextNodeIds;

	PxU32* mBitmapWords[eMAX_NB_BITMAPS];
	PxU32 mBitmapWordCounts[eMAX_NB_BITMAPS];
	PxU32 mBitmapBuffers[eMAX_NB_BITMAPS][sizeof(Cm::BitMap)];
	Cm::BitMap* mBitmaps[eMAX_NB_BITMAPS];


#ifndef __SPU__
	virtual void resize(const PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity>0);
		PX_ASSERT(newCapacity>mCapacity);

		const PxU32 nodeByteSize=(((sizeof(Node)*newCapacity) + 15) & ~15);
		const PxU32 nodeTypeByteSize=(((sizeof(NodeType)*newCapacity) + 15) & ~15);
		const PxU32 nodeBitMapWordCount = (((newCapacity + 31) & ~31) >> 5);
		const PxU32 nodeBitmapWordByteSize=(((sizeof(PxU32)*nodeBitMapWordCount) + 15) & ~15);

		const PxU32 byteSize=(nodeByteSize + nodeTypeByteSize + nodeTypeByteSize + nodeBitmapWordByteSize*eMAX_NB_BITMAPS);
		PxU8* newBuffer=(PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("IslandManager"));

		PxU32 offset=0;
		Node* newElems=(Node*)(newBuffer + offset);
		offset+=nodeByteSize;
		NodeType* newFreeElems=(NodeType*)(newBuffer + offset);
		offset+=nodeTypeByteSize;
		NodeType* newNextNodeIds=(NodeType*)(newBuffer + offset);
		offset+=nodeTypeByteSize;
		PxU32* newBitMapWords[eMAX_NB_BITMAPS];
		for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
		{
			newBitMapWords[i] = (PxU32*)(newBuffer + offset);
			PxMemZero(newBitMapWords[i], nodeBitmapWordByteSize);
			offset += nodeBitmapWordByteSize;
		}
		PX_ASSERT(byteSize==offset);

		if(mElems)
		{
			PxMemCopy(newElems, mElems, sizeof(Node)*mCapacity);
			PxMemCopy(newFreeElems, mFreeElems, sizeof(NodeType)*mCapacity);
			PxMemCopy(newNextNodeIds, mNextNodeIds, sizeof(NodeType)*mCapacity);
			for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
			{
				PxMemCopy(newBitMapWords[i], mBitmapWords[i], sizeof(PxU32)*mBitmapWordCounts[i]);
			}
			PX_FREE(mElems);
		}

		mElems=newElems;
		mFreeElems=newFreeElems;
		setupNewFreeElems(mCapacity,newCapacity);
		PX_ASSERT(getNumAvailableElems() == computeNumAvailableElems());

		PxMemSet(&newNextNodeIds[mCapacity],0xff,sizeof(NodeType)*(newCapacity-mCapacity));
		mNextNodeIds=newNextNodeIds;

		for(PxU32 i = 0; i < eMAX_NB_BITMAPS; i++)
		{
			mBitmapWords[i] = newBitMapWords[i];
			mBitmapWordCounts[i] = nodeBitMapWordCount;
			mBitmaps[i]->setWords(newBitMapWords[i],nodeBitMapWordCount);
		}

		//Set the new capacity.
		mCapacity=newCapacity;
	}
#endif
};

class EdgeManager : public ElemManager<Edge,EdgeType>
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

	EdgeManager() 
		: ElemManager<Edge,EdgeType>(),
		mNextEdgeIds(NULL)
	{
	}

#ifdef __SPU__
	void set(Edge* allEdges, EdgeType* freeEdges, const EdgeType nextFreeElem, const PxU32 numFreeElems, const PxU32 capacity, EdgeType* nextEdgeIds)
	{
		ElemManager<Edge,EdgeType>::set(allEdges,freeEdges,nextFreeElem,numFreeElems,capacity);
		mNextEdgeIds=nextEdgeIds;
	}
#endif

#ifndef __SPU__
	virtual ~EdgeManager()
	{
		PX_FREE(mElems);
	}
#endif

	EdgeType* getNextEdgeIds() {return mNextEdgeIds;}
	const EdgeType* getNextEdgeIds() const {return mNextEdgeIds;}

private:

	EdgeType* mNextEdgeIds;

#ifndef __SPU__
	virtual void resize(const PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity>0);
		PX_ASSERT(newCapacity>mCapacity);

		const PxU32 edgeByteSize=(((sizeof(Edge)*newCapacity) + 15) & ~15);
		const PxU32 edgeTypeByteSize=(((sizeof(EdgeType)*newCapacity) + 15) & ~15);
		const PxU32 byteSize=(edgeByteSize + edgeTypeByteSize + edgeTypeByteSize);
		PxU8* newBuffer=(PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("IslandManager"));

		Edge* newElems=(Edge*)newBuffer;
		EdgeType* newFreeElems=(EdgeType*)(newBuffer + edgeByteSize);
		EdgeType* newNextEdgeIds=(EdgeType*)(newBuffer + edgeByteSize + edgeTypeByteSize);

		if(mElems)
		{
			PxMemCopy(newElems, mElems, sizeof(Edge)*mCapacity);
			PxMemCopy(newFreeElems, mFreeElems, sizeof(EdgeType)*mCapacity);
			PxMemCopy(newNextEdgeIds, mNextEdgeIds, sizeof(EdgeType)*mCapacity);
			PX_FREE(mElems);
		}

		mElems=newElems;
		mFreeElems=newFreeElems;
		mNextEdgeIds=newNextEdgeIds;

		setupNewFreeElems(mCapacity,newCapacity);
		PxMemSet(&mNextEdgeIds[mCapacity],0xff,sizeof(EdgeType)*(newCapacity-mCapacity));

		//Set the new capacity.
		mCapacity=newCapacity;
	}
#endif
};

class IslandManager : public ElemManager<Island,IslandType>
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

	IslandManager() 
		: ElemManager<Island,IslandType>(),
		mBitmapWords(NULL),
		mBitmapWordCount(0)
	{
		mBitMap = (Cm::BitMap*)mbmBuffer;
		new(mBitMap) Cm::BitMap();
	}

#ifdef __SPU__
	void set(Island* allIslands, IslandType* freeIslands, const IslandType nextFreeElem, const PxU32 numFreeElems, const PxU32 capacity, PxU32* islandsBitmapWords, const PxU32 islandsBitmapWordCount)
	{
		ElemManager<Island,IslandType>::set(allIslands,freeIslands,nextFreeElem,numFreeElems,capacity);
		mBitmapWords=islandsBitmapWords;
		mBitmapWordCount=islandsBitmapWordCount;
		mBitMap->setWords(islandsBitmapWords,islandsBitmapWordCount);
	}
#endif

#ifndef __SPU__
	virtual ~IslandManager()
	{
		PX_FREE(mElems);
	}
#endif

	PX_FORCE_INLINE Cm::BitMap& getBitmap() {return *mBitMap;}
	PX_FORCE_INLINE const Cm::BitMap& getBitmap() const {return *mBitMap;}
#ifndef __SPU__
	virtual void resize(const PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity>0);
		PX_ASSERT(newCapacity>mCapacity);
		PX_ASSERT(mBitmapWordCount == (mCapacity>>5));

		const PxU32 newBitmapWordCount = (newCapacity>>5);

		const PxU32 islandByteSize=(((sizeof(Island)*newCapacity) + 15) & ~15);
		const PxU32 islandTypeByteSize=(((sizeof(IslandType)*newCapacity) + 15) & ~15);
		const PxU32 islandBitmapWordsByteSize=(((sizeof(PxU32)*newBitmapWordCount) + 15) & ~15);
		const PxU32 byteSize=(islandByteSize + islandTypeByteSize + islandBitmapWordsByteSize);
		PxU8* newBuffer=(PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("IslandManager"));

		Island* newElems=(Island*)newBuffer;
		IslandType* newFreeElems=(IslandType*)(newBuffer + islandByteSize);
		PxU32* newBitmapWords=(PxU32*)(newBuffer + islandByteSize + islandTypeByteSize);

		if(mElems)
		{
			PxMemCopy(newElems, mElems, sizeof(Island)*mCapacity);
			PxMemCopy(newFreeElems, mFreeElems, sizeof(IslandType)*mCapacity);
			PxMemCopy(newBitmapWords, mBitmapWords, sizeof(PxU32)*mBitmapWordCount);
			PX_FREE(mElems);
		}

		mElems=newElems;
		mFreeElems=newFreeElems;
		setupNewFreeElems(mCapacity,newCapacity);

		PxMemZero(&newBitmapWords[mBitmapWordCount],sizeof(PxU32)*(newBitmapWordCount-mBitmapWordCount));
		mBitMap->setWords(newBitmapWords,newBitmapWordCount);
		mBitmapWords=newBitmapWords;
		mBitmapWordCount=newBitmapWordCount;

		//Set the new capacity.
		mCapacity=newCapacity;
	}
#endif

#ifdef PX_DEBUG
	PxU32 getNumUsedIslands() const;
	PxU32 getNumFreeIslands() const;
#endif

private:

	PxU8 mbmBuffer[sizeof(Cm::BitMap)];
	Cm::BitMap* mBitMap;
	PxU32* mBitmapWords;
	PxU32 mBitmapWordCount;
};


class ArticulationRootManager : public ElemManager<ArticulationRoot,NodeType>
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif


#ifdef __SPU__
	void set(ArticulationRoot* allArtics, const PxU32 capacity)
	{
		ElemManager<ArticulationRoot,NodeType>::set(allArtics,NULL,0,0,capacity);
	}
#endif

#ifndef __SPU__

	virtual ~ArticulationRootManager()
	{
		PX_FREE(mElems);
	}

	virtual void resize(const PxU32 newCapacity)
	{
		PX_ASSERT(newCapacity>0);
		PX_ASSERT(newCapacity>mCapacity);

		const PxU32 nodeByteSize=(((sizeof(ArticulationRoot)*newCapacity) + 15) & ~15);
		const PxU32 nodeTypeByteSize=(((sizeof(NodeType)*newCapacity) + 15) & ~15);
		const PxU32 byteSize=(nodeByteSize + nodeTypeByteSize );
		PxU8* newBuffer=(PxU8*)PX_ALLOC(byteSize, PX_DEBUG_EXP("IslandManager"));

		ArticulationRoot* newElems=(ArticulationRoot*)newBuffer;
		NodeType* newFreeElems=(NodeType*)(newBuffer + nodeByteSize);

		if(mElems)
		{
			PxMemCopy(newElems, mElems, sizeof(ArticulationRoot)*mCapacity);
			PxMemCopy(newFreeElems, mFreeElems, sizeof(NodeType)*mCapacity);
			PX_FREE(mElems);
		}

		mElems=newElems;
		mFreeElems=newFreeElems;

		setupNewFreeElems(mCapacity,newCapacity);

		//Set the new capacity.
		mCapacity=newCapacity;
	}
#endif
};

struct ProcessSleepingIslandsComputeData
{
	//All the data of this struct is allocated from a single memory block. The following members describe that block
	//such that it can be shared for other purposes if appropriate.
	PxU8* mDataBlock;
	PxU32 mDataBlockSize;

	//Nodes whose states have changed from in a sleeping island to not in a sleeping island or vice versa.
	PxU8** mBodiesToWakeOrSleep;
	PxU32 mBodiesToWakeSize;
	PxU32 mBodiesToSleepSize;
	PxU32 mBodiesToWakeOrSleepCapacity;

	//Body pairs that require a second pass through np because both bodies have 
	//been woken up in updateIslands and now need contact information for the solver.
	NarrowPhaseContactManager* mNarrowPhaseContactManagers;
	PxU32 mNarrowPhaseContactManagersSize;
	PxU32 mNarrowPhaseContactManagersCapacity;

	//Map between entry in mNodeManager and entry in mSolverBodies
	NodeType* mSolverBodyMap;
	PxU32 mSolverBodyMapCapacity;

	//Outputs passed to solver
	//Array of kinematics
	PxsRigidBody** mSolverKinematics;
	PxU32 mSolverKinematicsSize;
	PxU32 mSolverKinematicsCapacity;
	//Array of bodies
	PxsRigidBody** mSolverBodies;
	PxU32 mSolverBodiesSize;
	PxU32 mSolverBodiesCapacity;
	//Array of articulations
	PxsArticulation** mSolverArticulations;
	void** mSolverArticulationOwners;
	PxU32 mSolverArticulationsSize;
	PxU32 mSolverArticulationsCapacity;
	//Array of contact managers.
	PxsIndexedContactManager* mSolverContactManagers;
	PxU32 mSolverContactManagersSize;
	PxU32 mSolverContactManagersCapacity;
	//Array of constraints
	PxsIndexedConstraint* mSolverConstraints;
	PxU32 mSolverConstraintsSize;
	PxU32 mSolverConstraintsCapacity;
	//Indices of start point of each awake island's bodies/articulations/contactmanagers/constraints
	//Note: This is also used as a buffer to track islands that need a second island generation pass.
	//      The valid islands are filled in from the array start, the islands that need postprocessing are filled in from the array end. Once the
	//      postprocessing is done, the corresponding islands will get appended at the end of the valid islands.
	PxsIslandIndices* mIslandIndices;
	PxU32 mIslandIndicesSize;
	PxU32 mIslandIndicesCapacity;
	PxU32 mIslandIndicesSecondPassSize;  // number of islands that need a second pass. Start at the array end and get filled in towards the array start.
};

struct IslandManagerUpdateWorkBuffers
{
	//These buffers need to persist across the entire island manager update.
	//Buffer to store source node ids for each kinematic proxy.
	NodeType* mKinematicProxySourceNodeIds;
	//Buffer to store the proxy nodes ids for each kinematic.
	NodeType* mKinematicProxyNextNodeIds;
	NodeType* mKinematicProxyLastNodeIds;

	//These buffers don't need to persist across the entire island manager update.

	//Bitmap of islands containing edges that have been disconnected or deleted.
	//Bitmap of islands that need processed in processSleepingIslands
	enum 
	{
		eBROKEN_EDGE_ISLANDS = 0,
		ePROCESS_ISLANDS,
		eCHANGED_NODES,
		eMAX_NB_BITMAPS
	};
	PxU32* mBitmapWords[eMAX_NB_BITMAPS];
	PxU32 mBitmapWordCount[eMAX_NB_BITMAPS];
	PxU8 mBitmapBuffer[eMAX_NB_BITMAPS][sizeof(Cm::BitMap)];
	Cm::BitMap* mBitmap[eMAX_NB_BITMAPS];

	//Buffer to store a copy of the nodes in each island.
	NodeType* mGraphNextNodes;
	//Buffers to store graph of island merging.
	IslandType* mGraphStartIslands;
	IslandType* mGraphNextIslands;
};

// necessary because else the Wii U linker fails (function call is too far away). Revisit when new compiler version is out
#ifdef __SPU__
void processSleepingIslands
(const Cm::BitMap& islandsToUpdate, const PxU32 rigidBodyOffset, 
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager, 
 const NodeType* kinematicSourceNodeIds, 
 ProcessSleepingIslandsComputeData& psicData);

void processSleepingIslandsSecondPass
(const Cm::BitMap& islandsToUpdate, const PxU32 rigidBodyOffset, 
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager, 
 const NodeType* kinematicSourceNodeIds, 
 ProcessSleepingIslandsComputeData& psicData);
#endif


void updateIslandsMain
(const PxU32 rigidBodyOffset,
 const NodeType* PX_RESTRICT deletedNodes, const PxU32 numDeletedNodes,
 const NodeType* PX_RESTRICT createdNodes, const PxU32 numCreatedNodes,
 const EdgeType* PX_RESTRICT deletedEdges, const PxU32 numDeletedEdges,
 const EdgeType* PX_RESTRICT createdEdges, const PxU32 numCreatedEdges,
 const EdgeType* PX_RESTRICT brokenEdges, const PxU32 numBrokenEdges,
 const EdgeType* PX_RESTRICT joinedEdges, const PxU32 numJoinedEdges,
 const Cm::BitMap& kinematicNodesBitmap, const Cm::BitMap& kinematicChangeNodesBitmap, const PxU32 numKinematics,
 const Cm::BitMap& notReadyForSleepingNodesBitmap, const Cm::BitMap& notReadyForSleepingChangeNodesBitmap,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager,
 ProcessSleepingIslandsComputeData& psicData,
 IslandManagerUpdateWorkBuffers& workBuffers,
 Cm::EventProfiler* eventProfiler=NULL);

void updateIslandsSecondPassMain
(const PxU32 rigidBodyOffset, Cm::BitMap& affectedIslandsBitmap,
 const EdgeType* PX_RESTRICT brokenEdges, const PxU32 numBrokenEdges,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager,
 ProcessSleepingIslandsComputeData& psicData,
 IslandManagerUpdateWorkBuffers& workBuffers,
 Cm::EventProfiler* eventProfiler=NULL);

void mergeKinematicProxiesBackToSource
(const Cm::BitMap& kinematicNodesBitmap,
 const NodeType* PX_RESTRICT kinematicProxySourceNodes, const NodeType* PX_RESTRICT kinematicProxyNextNodes,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
 Cm::BitMap& kinematicIslandsBitmap,
 IslandType* graphStartIslands, IslandType* graphNextIslands);

#ifndef __SPU__

class NodeChangeManager
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

	NodeChangeManager()
		: mCreatedNodes(NULL),
		  mCreatedNodesSize(0),
		  mDeletedNodes(NULL),
		  mDeletedNodesSize(0),
		  mCapacity(0),
		  mDefaultCapacity(0)
	{
	}

	~NodeChangeManager()
	{
		if(mCreatedNodes) PX_FREE(mCreatedNodes);
	}

	void init(const PxU32 size)
	{
		if(mDefaultCapacity)
		{
			if(size > mCapacity)
			{
				resize(size);
				mDefaultCapacity = size;
			}
		}
		else
		{
			preallocate(size);
		}
	}

	void preallocate(const PxU32 size)
	{
		mDefaultCapacity=size;

		NodeType* arrays=(NodeType*)PX_ALLOC(sizeof(NodeType)*2*mDefaultCapacity,PX_DEBUG_EXP("IslandManager"));

		PX_ASSERT(NULL==mCreatedNodes);
		mCreatedNodes=arrays + 0*mDefaultCapacity;
		PX_ASSERT(0==mCreatedNodesSize);

		PX_ASSERT(NULL==mDeletedNodes);
		mDeletedNodes=arrays + 1*mDefaultCapacity;
		PX_ASSERT(0==mDeletedNodesSize);

		mCapacity=mDefaultCapacity;
	}

	PX_FORCE_INLINE PxU32 getNumCreatedNodes() const {return mCreatedNodesSize;}
	PX_FORCE_INLINE PxU32 getNumDeletedNodes() const {return mDeletedNodesSize;}

	PX_FORCE_INLINE const NodeType* getCreatedNodes() const {return mCreatedNodes;}
	PX_FORCE_INLINE const NodeType* getDeletedNodes() const {return mDeletedNodes;}

	void addCreatedNode(const NodeType id)
	{
		if(mCreatedNodesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mCreatedNodes[mCreatedNodesSize]=id;
		mCreatedNodesSize++;
	}

	void addDeletedNode(const NodeType id)
	{
		if(mDeletedNodesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mDeletedNodes[mDeletedNodesSize]=id;
		mDeletedNodesSize++;
	}

	void reset()
	{
		mCreatedNodesSize=0;
		mDeletedNodesSize=0;

		if(mCapacity>mDefaultCapacity)
		{
			PX_ASSERT(mCreatedNodes);
			PX_ASSERT(mDeletedNodes);
			PX_FREE(mCreatedNodes);
			mCreatedNodes=NULL;
			mDeletedNodes=NULL;
			preallocate(mDefaultCapacity);
		}
	}

private:

	NodeType* mCreatedNodes;		//reported from high-level
	PxU32 mCreatedNodesSize;
	NodeType* mDeletedNodes;		//reported from high-level
	PxU32 mDeletedNodesSize;
	PxU32 mCapacity;
	PxU32 mDefaultCapacity;

	void resize(PxU32 nb)
	{
		NodeType* arrays=(NodeType*)PX_ALLOC(sizeof(NodeType)*2*nb,PX_DEBUG_EXP("IslandManager"));

		NodeType* newCreatedNodes=arrays + 0*nb;
		NodeType* newDeletedNodes=arrays + 1*nb;

		PxMemCopy(newCreatedNodes, mCreatedNodes, sizeof(NodeType)*mCreatedNodesSize);
		PxMemCopy(newDeletedNodes, mDeletedNodes, sizeof(NodeType)*mDeletedNodesSize);

		PX_FREE(mCreatedNodes);

		mCreatedNodes=newCreatedNodes;
		mDeletedNodes=newDeletedNodes;

		mCapacity = nb;
	}
};

class EdgeChangeManager
{
public:

#ifdef PX_PS3
	friend class IslandGenSpuTask;
#endif

	EdgeChangeManager()
		: mCreatedEdges(NULL),
		  mCreatedEdgesSize(0),
		  mDeletedEdges(NULL),
		  mDeletedEdgesSize(0),
		  mBrokenEdges(NULL),
		  mBrokenEdgesSize(0),
		  mJoinedEdges(NULL),
		  mJoinedEdgesSize(0),
		  mCapacity(0),
		  mDefaultCapacity(0)
	{
	}

	~EdgeChangeManager()
	{
		if(mCreatedEdges) PX_FREE(mCreatedEdges);
	}

	void init(const PxU32 size)
	{
		if(mDefaultCapacity)
		{
			if(size > mCapacity)
			{
				resize(size);
				mDefaultCapacity = size;
			}
		}
		else
		{
			preallocate(size);
		}
	}

	void preallocate(const PxU32 size)
	{
		mDefaultCapacity=size;

		EdgeType* arrays=(EdgeType*)PX_ALLOC(sizeof(EdgeType)*4*mDefaultCapacity,PX_DEBUG_EXP("IslandManager"));

		PX_ASSERT(NULL==mCreatedEdges);
		mCreatedEdges=arrays + 0*mDefaultCapacity;
		PX_ASSERT(0==mCreatedEdgesSize);

		PX_ASSERT(NULL==mDeletedEdges);
		mDeletedEdges=arrays + 1*mDefaultCapacity;
		PX_ASSERT(0==mDeletedEdgesSize);

		PX_ASSERT(NULL==mJoinedEdges);
		mJoinedEdges=arrays + 3*mDefaultCapacity;
		PX_ASSERT(0==mJoinedEdgesSize);

		PX_ASSERT(NULL==mBrokenEdges);
		mBrokenEdges=arrays + 2*mDefaultCapacity;
		PX_ASSERT(0==mBrokenEdgesSize);

		mCapacity=mDefaultCapacity;
	}

	PX_FORCE_INLINE PxU32 getNumCreatedEdges() const {return mCreatedEdgesSize;}
	PX_FORCE_INLINE PxU32 getNumDeletedEdges() const {return mDeletedEdgesSize;}
	PX_FORCE_INLINE PxU32 getNumJoinedEdges() const {return mJoinedEdgesSize;}
	PX_FORCE_INLINE PxU32 getNumBrokenEdges() const {return mBrokenEdgesSize;}

	PX_FORCE_INLINE const EdgeType* getCreatedEdges() const {return mCreatedEdges;}
	PX_FORCE_INLINE const EdgeType* getDeletedEdges() const {return mDeletedEdges;}
	PX_FORCE_INLINE const EdgeType* getJoinedEdges() const {return mJoinedEdges;}
	PX_FORCE_INLINE const EdgeType* getBrokenEdges() const {return mBrokenEdges;}

	void addCreatedEdge(const EdgeType id)
	{
		if(mCreatedEdgesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mCreatedEdges[mCreatedEdgesSize]=id;
		mCreatedEdgesSize++;
	}

	void addDeletedEdge(const EdgeType id)
	{
		if(mDeletedEdgesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mDeletedEdges[mDeletedEdgesSize]=id;
		mDeletedEdgesSize++;
	}

	void addJoinedEdge(const EdgeType id)
	{
		if(mJoinedEdgesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mJoinedEdges[mJoinedEdgesSize]=id;
		mJoinedEdgesSize++;
	}

	void addBrokenEdge(const EdgeType id)
	{
		if(mBrokenEdgesSize==mCapacity)
		{
			resize(2*mCapacity);
		}
		mBrokenEdges[mBrokenEdgesSize]=id;
		mBrokenEdgesSize++;
	}

	void cleanupEdgeEvents(PxI32* edgeEventWorkBuffer, const PxU32 entryCapacity);
	void cleanupBrokenEdgeEvents(const Edge* allEdges);
	PX_FORCE_INLINE void clearJoinedEdgeEvents() { mJoinedEdgesSize=0; }
	PX_FORCE_INLINE void clearBrokenEdgeEvents() { mBrokenEdgesSize=0; }

	void reset()
	{
		mCreatedEdgesSize=0;
		mDeletedEdgesSize=0;
		mJoinedEdgesSize=0;
		mBrokenEdgesSize=0;

		if(mCapacity>mDefaultCapacity)
		{
			PX_ASSERT(mCreatedEdges);
			PX_ASSERT(mDeletedEdges);
			PX_ASSERT(mJoinedEdges);
			PX_ASSERT(mBrokenEdges);
			PX_FREE(mCreatedEdges);
			mCreatedEdges=NULL;
			mDeletedEdges=NULL;
			mJoinedEdges=NULL;
			mBrokenEdges=NULL;
			preallocate(mDefaultCapacity);
		}
	}

private:

	EdgeType* mCreatedEdges;		//reported from broadphase
	PxU32 mCreatedEdgesSize;
	EdgeType* mDeletedEdges;		//reported from broadphase
	PxU32 mDeletedEdgesSize;
	EdgeType* mBrokenEdges;		//reported from touch lost
	PxU32 mBrokenEdgesSize;
	EdgeType* mJoinedEdges;		//reported from touch lost
	PxU32 mJoinedEdgesSize;
	PxU32 mCapacity;
	PxU32 mDefaultCapacity;

	void resize(const PxU32 nb)
	{
		EdgeType* arrays=(EdgeType*)PX_ALLOC(sizeof(EdgeType)*4*nb,PX_DEBUG_EXP("IslandManager"));

		EdgeType* newCreatedEdges=arrays + 0*nb;
		EdgeType* newDeletedEdges=arrays + 1*nb;
		EdgeType* newJoinedEdges=arrays + 2*nb;
		EdgeType* newBrokenEdges=arrays + 3*nb;

		PxMemCopy(newCreatedEdges, mCreatedEdges, sizeof(EdgeType)*mCreatedEdgesSize);
		PxMemCopy(newDeletedEdges, mDeletedEdges, sizeof(EdgeType)*mDeletedEdgesSize);
		PxMemCopy(newJoinedEdges, mJoinedEdges, sizeof(EdgeType)*mJoinedEdgesSize);
		PxMemCopy(newBrokenEdges, mBrokenEdges, sizeof(EdgeType)*mBrokenEdgesSize);

		PX_FREE(mCreatedEdges);

		mCreatedEdges=newCreatedEdges;
		mDeletedEdges=newDeletedEdges;
		mJoinedEdges=newJoinedEdges;
		mBrokenEdges=newBrokenEdges;

		mCapacity = nb;
	}
};

#endif //__SPU__

} //namespace physx


#endif //PXS_ISLAND_MANAGER_AUX_H
