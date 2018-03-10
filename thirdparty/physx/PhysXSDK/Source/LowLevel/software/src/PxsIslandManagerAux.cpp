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

#include "PxsIslandManagerAux.h"

using namespace physx;

#include "PxsRigidBody.h"
#include "PxvDynamics.h"
#include "PxsArticulation.h"
#include "CmEventProfiler.h"
#include "PxProfileEventId.h"

#ifndef __SPU__
void EdgeChangeManager::cleanupEdgeEvents(PxI32* edgeEventWorkBuffer, const PxU32 entryCapacity)
{
	PX_UNUSED(entryCapacity);

	//
	//The same edge can contain duplicate and competing events. For example, if the user does a sequence of wakeUp and putToSleep calls without
	//every simulating or if the user puts an object to sleep which then gets hit by an awake object.
	//The following code removes duplicates and competing events.
	//

	const PxU32 oldBrokenEdgesSize = mBrokenEdgesSize;
	const PxU32 oldJoinedEdgesSize = mJoinedEdgesSize;
	PX_ASSERT(oldBrokenEdgesSize && oldJoinedEdgesSize);  // else do not call this method

	EdgeType* PX_RESTRICT brokenEdges = mBrokenEdges;
	EdgeType* PX_RESTRICT joinedEdges = mJoinedEdges;

	bool cleanupNeeded = false;
	for(PxU32 i=0; i < oldBrokenEdgesSize; i++)
	{
		const EdgeType edgeId = brokenEdges[i];
		PX_ASSERT(edgeId < entryCapacity);
		cleanupNeeded |= (edgeEventWorkBuffer[edgeId] != 0);
		edgeEventWorkBuffer[edgeId]--;
	}

	for(PxU32 i=0; i < oldJoinedEdgesSize; i++)
	{
		const EdgeType edgeId = joinedEdges[i];
		PX_ASSERT(edgeId < entryCapacity);
		cleanupNeeded |= (edgeEventWorkBuffer[edgeId] != 0);
		edgeEventWorkBuffer[edgeId]++;
	}

	if (cleanupNeeded)
	{
		//
		// edge event count:
		//  0: There were the same number of connect/unconnect events for the edge -> they cancel each other out, so ignore events for this edge
		//  1: There was one more connect event than unconnect event -> record one connect event
		// -1: There was one more unconnect event than connect event -> record one unconnect event
		//

		PxU32 newEdgeEventCount = 0;
		for(PxU32 i=0; i < oldBrokenEdgesSize; i++)
		{
			const EdgeType edgeId = brokenEdges[i];
			PX_ASSERT((edgeEventWorkBuffer[edgeId] <= 1) || (edgeEventWorkBuffer[edgeId] >= -1));

			if (edgeEventWorkBuffer[edgeId] < 0)
			{
				brokenEdges[newEdgeEventCount] = brokenEdges[i];
				newEdgeEventCount++;
				edgeEventWorkBuffer[edgeId] = 0;  // make sure this is the only event recorded for this edge
			}
		}
		mBrokenEdgesSize = newEdgeEventCount;

		newEdgeEventCount = 0;
		for(PxU32 i=0; i < oldJoinedEdgesSize; i++)
		{
			const EdgeType edgeId = joinedEdges[i];
			PX_ASSERT((edgeEventWorkBuffer[edgeId] <= 1) || (edgeEventWorkBuffer[edgeId] >= -1));

			if (edgeEventWorkBuffer[edgeId] > 0)
			{
				joinedEdges[newEdgeEventCount] = joinedEdges[i];
				newEdgeEventCount++;
				edgeEventWorkBuffer[edgeId] = 0;  // make sure this is the only event recorded for this edge
			}
		}
		mJoinedEdgesSize = newEdgeEventCount;
	}
}

void EdgeChangeManager::cleanupBrokenEdgeEvents(const Edge* allEdges)
{
	//
	//only applicable for second island gen pass because there we know that only unconnect events can make it through.
	//If an object did not have touch before activation and has touch after activation, then there will be an unconnect
	//(triggered by the activation) followed by a connect event (triggered by narrowphase finding a touch). Those events
	//should cancel each other out and the current state will be the same as the state before activation which, for second 
	//pass edges, means connected. Hence, this case can be detected by finding edges in the broken edge event list
	//which are connected.
	//

	if (mJoinedEdgesSize)  // else the problem can not occur
	{
		const Edge* PX_RESTRICT edges = allEdges;
		EdgeType* PX_RESTRICT brokenEdges = mBrokenEdges;
		const PxU32 oldBrokenEdgesSize = mBrokenEdgesSize;
		PxU32 newBrokenEdgesSize = 0;
		for(PxU32 i=0; i < oldBrokenEdgesSize; i++)
		{
			const EdgeType edgeId = brokenEdges[i];
			const Edge& edge = edges[edgeId];

			if (!edge.getIsConnected())
			{
				brokenEdges[newBrokenEdgesSize] = brokenEdges[i];
				newBrokenEdgesSize++;
			}
		}
		PX_ASSERT(mJoinedEdgesSize == (mBrokenEdgesSize - newBrokenEdgesSize));  //in the second island gen pass, there should be no join events apart from the ones described here

		mBrokenEdgesSize = newBrokenEdgesSize;

		mJoinedEdgesSize = 0;
	}
}
#endif

//b |= a
//Necessary for spu because Cm::BitMap::combine pulls in the allocator.
static PX_FORCE_INLINE void combineOr(const Cm::BitMap& a, Cm::BitMap& b)
{
	PX_ASSERT(a.getWordCount() == b.getWordCount());
	const PxU32* awords = a.getWords();
	PxU32* bwords = b.getWords();
	const PxU32 count = a.getWordCount();
	for(PxU32 i = 0; i < count; i++)
	{
		bwords[i] |= awords[i];
	}
}

static PX_FORCE_INLINE void releaseIsland(const IslandType islandId, IslandManager& islands)
{
	islands.release(islandId);
	PX_ASSERT(islands.getBitmap().test(islandId));
	islands.getBitmap().reset(islandId);
}

static PX_FORCE_INLINE IslandType getNewIsland(IslandManager& islands)
{
	const IslandType newIslandId=islands.getAvailableElemNoResize();
	PX_ASSERT(!islands.getBitmap().test(newIslandId));
	islands.getBitmap().set(newIslandId);
	return newIslandId;
}

static PX_FORCE_INLINE void addEdgeToIsland
(const IslandType islandId, const EdgeType edgeId, 
 EdgeType* PX_RESTRICT nextEdgeIds, const PxU32 allEdgesCapacity, 
 IslandManager& islands)
{
	//Add the edge to the island.
	Island& island=islands.get(islandId);
	PX_ASSERT(edgeId<allEdgesCapacity);
	PX_ASSERT(edgeId!=island.mStartEdgeId);
	PX_UNUSED(allEdgesCapacity);

	nextEdgeIds[edgeId]=island.mStartEdgeId;
	island.mStartEdgeId=edgeId;
	island.mEndEdgeId=(INVALID_EDGE==island.mEndEdgeId ? edgeId : island.mEndEdgeId);
}

static PX_FORCE_INLINE void addNodeToIsland
(const IslandType islandId, const NodeType nodeId,
 Node* PX_RESTRICT allNodes, NodeType* PX_RESTRICT nextNodeIds, const PxU32 allNodesCapacity, 
 IslandManager& islands)
{
	PX_ASSERT(islands.getBitmap().test(islandId));
	PX_ASSERT(nodeId<allNodesCapacity);
	PX_ASSERT(!allNodes[nodeId].getIsDeleted());
	PX_UNUSED(allNodesCapacity);

	//Get the island and the node to be added to the island.
	Node& node=allNodes[nodeId];
	Island& island=islands.get(islandId);

	//Set the island of the node.
	node.setIslandId(islandId);

	//Connect the linked list of nodes
	PX_ASSERT(nodeId!=island.mStartNodeId);
	nextNodeIds[nodeId]=island.mStartNodeId;
	island.mStartNodeId=nodeId;
	island.mEndNodeId=(INVALID_NODE==island.mEndNodeId ? nodeId : island.mEndNodeId);
}

static PX_FORCE_INLINE void joinIslands
(const IslandType islandId1, const IslandType islandId2,
 Node* PX_RESTRICT /*allNodes*/, const PxU32 /*allNodesCapacity*/,
 Edge* PX_RESTRICT /*allEdges*/, const PxU32 /*allEdgesCapacity*/,
 NodeType* nextNodeIds, EdgeType* nextEdgeIds,
 IslandManager& islands)
{
	PX_ASSERT(islandId1!=islandId2);

	Island* PX_RESTRICT allIslands=islands.getAll();

	//Get both islands.
	PX_ASSERT(islands.getBitmap().test(islandId1));
	PX_ASSERT(islandId1<islands.getCapacity());
	Island& island1=allIslands[islandId1];
	PX_ASSERT(islands.getBitmap().test(islandId2));
	PX_ASSERT(islandId2<islands.getCapacity());
	Island& island2=allIslands[islandId2];

	//Join the list of edges together.
	//If Island2 has no edges then the list of edges is just the edges in island1 (so there is nothing to in this case).
	//If island1 has no edges then the list of edges is just the edges in island2.
	if(INVALID_EDGE==island1.mStartEdgeId)
	{
		//Island1 has no edges so the list of edges is just the edges in island 2.
		PX_ASSERT(INVALID_EDGE==island1.mEndEdgeId);
		island1.mStartEdgeId=island2.mStartEdgeId;
		island1.mEndEdgeId=island2.mEndEdgeId;
	}
	else if(INVALID_EDGE!=island2.mStartEdgeId)
	{
		//Both island1 and island2 have edges.
		PX_ASSERT(INVALID_EDGE!=island1.mStartEdgeId);
		PX_ASSERT(INVALID_EDGE!=island1.mEndEdgeId);
		PX_ASSERT(INVALID_EDGE!=island2.mStartEdgeId);
		PX_ASSERT(INVALID_EDGE!=island2.mEndEdgeId);
		PX_ASSERT(island1.mEndEdgeId!=island2.mStartEdgeId);
		nextEdgeIds[island1.mEndEdgeId]=island2.mStartEdgeId;
		island1.mEndEdgeId=island2.mEndEdgeId;
	}

	//Join the list of nodes together.
	//If Island2 has no nodes then the list of nodes is just the nodes in island1 (so there is nothing to in this case).
	//If island1 has no nodes then the list of nodes is just the nodes in island2.
	if(INVALID_NODE==island1.mStartNodeId)
	{
		//Island1 has no nodes so the list of nodes is just the nodes in island 2.
		PX_ASSERT(INVALID_NODE==island1.mEndNodeId);
		island1.mStartNodeId=island2.mStartNodeId;
		island1.mEndNodeId=island2.mEndNodeId;
	}
	else if(INVALID_NODE!=island2.mStartNodeId)
	{
		//Both island1 and island2 have nodes.
		PX_ASSERT(INVALID_NODE!=island1.mStartNodeId);
		PX_ASSERT(INVALID_NODE!=island1.mEndNodeId);
		PX_ASSERT(INVALID_NODE!=island2.mStartNodeId);
		PX_ASSERT(INVALID_NODE!=island2.mEndNodeId);
		PX_ASSERT(island2.mStartNodeId!=island1.mEndNodeId);
		nextNodeIds[island1.mEndNodeId]=island2.mStartNodeId;
		island1.mEndNodeId=island2.mEndNodeId;
	}

	//remove island 2
	releaseIsland(islandId2,islands);
}

static PX_FORCE_INLINE void setNodeIslandIdsAndJoinIslands
(const IslandType islandId1, const IslandType islandId2,
 Node* PX_RESTRICT allNodes, const PxU32 allNodesCapacity,
 Edge* PX_RESTRICT allEdges, const PxU32 allEdgesCapacity,
 NodeType* nextNodeIds, EdgeType* nextEdgeIds,
 IslandManager& islands)
{
	PX_ASSERT(islandId1!=islandId2);

	//Get island 2
	Island* PX_RESTRICT allIslands=islands.getAll();
	PX_ASSERT(islands.getBitmap().test(islandId2));
	PX_ASSERT(islandId2<islands.getCapacity());
	Island& island2=allIslands[islandId2];

	//Set all nodes in island 2 to be in island 1.
	NodeType nextNode=island2.mStartNodeId;
	while(nextNode!=INVALID_NODE)
	{
		PX_ASSERT(nextNode<allNodesCapacity);
		allNodes[nextNode].setIslandId(islandId1);
		nextNode=nextNodeIds[nextNode];
	}

	joinIslands(islandId1,islandId2,allNodes,allNodesCapacity,allEdges,allEdgesCapacity,nextNodeIds,nextEdgeIds,islands);
}

static void removeDeletedNodesFromIsland
(const IslandType islandId, NodeManager& nodeManager, IslandManager& islands)
{
	PX_ASSERT(islands.getBitmap().test(islandId));
	Island& island=islands.get(islandId);

	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();

	PX_ASSERT(INVALID_NODE==island.mStartNodeId || island.mStartNodeId!=nextNodeIds[island.mStartNodeId]);

	//If the start node has been deleted then keep 
	//updating until we find a start node that isn't deleted.
	{
		NodeType nextNode=island.mStartNodeId;
		while(nextNode!=INVALID_NODE && allNodes[nextNode].getIsDeleted())
		{
			const NodeType currNode=nextNode;
			nextNode=nextNodeIds[currNode];
			nextNodeIds[currNode]=INVALID_NODE;
		}
		island.mStartNodeId=nextNode;
	}

	//Remove deleted nodes from the linked list.
	{
		NodeType endNode=island.mStartNodeId;
		NodeType undeletedNode=island.mStartNodeId;
		while(undeletedNode!=INVALID_NODE)
		{
			PX_ASSERT(!allNodes[undeletedNode].getIsDeleted());

			NodeType nextNode=nextNodeIds[undeletedNode];
			while(nextNode!=INVALID_NODE && allNodes[nextNode].getIsDeleted())
			{
				const NodeType currNode=nextNode;
				nextNode=nextNodeIds[nextNode];
				nextNodeIds[currNode]=INVALID_NODE;
			}
			nextNodeIds[undeletedNode]=nextNode;
			endNode=undeletedNode;
			undeletedNode=nextNode;
		}
		island.mEndNodeId=endNode;
	}

	PX_ASSERT(INVALID_NODE==island.mStartNodeId || island.mStartNodeId!=nextNodeIds[island.mStartNodeId]);
}

static void removeDeletedNodesFromIslands2
(const IslandType* islandsToUpdate, const PxU32 numIslandsToUpdate,
 NodeManager& nodeManager, IslandManager& islands,
 Cm::BitMap& emptyIslandsBitmap)
{
	//Remove broken edges from affected islands.
	//Remove deleted nodes from affected islands.
	for(PxU32 i=0;i<numIslandsToUpdate;i++)
	{
		//Get the island.
		const IslandType islandId=islandsToUpdate[i];
		removeDeletedNodesFromIsland(islandId,nodeManager,islands);

		PX_ASSERT(islands.getBitmap().test(islandId));
		Island& island=islands.get(islandId);
		if(INVALID_NODE==island.mEndNodeId)
		{
			PX_ASSERT(INVALID_NODE==island.mStartNodeId);
			emptyIslandsBitmap.set(islandId);
		}
	}
}

/**
\brief Nodes that have been marked as deleted are removed from their islands.
\param[in] deletedNodes is the array of node ids that have been deleted.
\param[in] numDeletedNodes is the number of nodes that have been deleted (the length of deletedNodes).
\param[in,out] nodeManager is array of all nodes and a mapping that allows us to iterate over nodes in an island given the first node in the island.
This mapping will be altered by deleting nodes from islands.
\param[in,out]  islands is an array of all islands and a bitmap of active islands. 
Removing a node can change the start node of an island and can leave an island with no nodes.  
\param[] deletedNodeIslandsBitmap is a scratch bitmap that can be cleared and re-used after the function exits.
deletedNodeIslandsBitmap should be cleared before calling this function.
\param[out] emptyIslandsBitmap is a bitmap of all islands that are empty as a consequence of removing the deleted nodes.
emptyIslandsBitmap should be cleared before calling this function.
*/
static void removeDeletedNodesFromIslands
(const NodeType* deletedNodes, const PxU32 numDeletedNodes,
 NodeManager& nodeManager, IslandManager& islands,
 Cm::BitMap& deletedNodeIslandsBitmap, Cm::BitMap& emptyIslandsBitmap)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();

	//Add to the list any islands that contain a deleted node.
	for(PxU32 i=0;i<numDeletedNodes;i++)
	{
		const NodeType nodeId=deletedNodes[i];
		PX_ASSERT(nodeId<nodeManager.getCapacity());
		const Node& node=allNodes[nodeId];
		//If a body was added after PxScene.simulate and before PxScene.fetchResults then 
		//the addition will be delayed and not processed until the end of fetchResults.
		//If this body is then released straight after PxScene.fetchResults then at the 
		//next PxScene.simulate we will have a body that has been both added and removed.
		//The removal must take precedence over the addition.
		if(node.getIsDeleted() && !node.getIsNew())
		{
			const IslandType islandId=node.getIslandId();
			PX_ASSERT(islandId!=INVALID_ISLAND);
			PX_ASSERT(islands.getBitmap().test(islandId));
			deletedNodeIslandsBitmap.set(islandId);
		}
	}

#define MAX_NUM_ISLANDS_TO_UPDATE 1024

	//Gather a simple list of all islands affected by a deleted node.
	IslandType islandsToUpdate[MAX_NUM_ISLANDS_TO_UPDATE];
	PxU32 numIslandsToUpdate=0;
	const PxU32 lastSetBit = deletedNodeIslandsBitmap.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = deletedNodeIslandsBitmap.getWords()[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
			PX_ASSERT(islands.getBitmap().test(islandId));

			if(numIslandsToUpdate<MAX_NUM_ISLANDS_TO_UPDATE)
			{
				islandsToUpdate[numIslandsToUpdate]=islandId;
				numIslandsToUpdate++;
			}
			else
			{
				removeDeletedNodesFromIslands2(islandsToUpdate,numIslandsToUpdate,nodeManager,islands,emptyIslandsBitmap);
				islandsToUpdate[0]=islandId;
				numIslandsToUpdate=1;
			}
		}
	}

	removeDeletedNodesFromIslands2(islandsToUpdate,numIslandsToUpdate,nodeManager,islands,emptyIslandsBitmap);
}

static void removeBrokenEdgesFromIslands2
(const IslandType* PX_RESTRICT islandsToUpdate, const PxU32 numIslandsToUpdate,
 EdgeManager& edgeManager, IslandManager& islands)
{
	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();

	//Remove broken edges from affected islands.
	//Remove deleted nodes from affected islands.
	for(PxU32 i=0;i<numIslandsToUpdate;i++)
	{
		//Get the island.
		const IslandType islandId=islandsToUpdate[i];
		PX_ASSERT(islands.getBitmap().test(islandId));
		Island& island=islands.get(islandId);

		PX_ASSERT(INVALID_EDGE==island.mStartEdgeId || island.mStartEdgeId!=nextEdgeIds[island.mStartEdgeId]);

		//If the start edge has been deleted then keep 
		//updating until we find a start edge that isn't deleted.
		{
			EdgeType nextEdge=island.mStartEdgeId;
			while(nextEdge!=INVALID_EDGE && !allEdges[nextEdge].getIsConnected())
			{
				const EdgeType currEdge=nextEdge;
				nextEdge=nextEdgeIds[currEdge];
				nextEdgeIds[currEdge]=INVALID_EDGE;
			}
			island.mStartEdgeId=nextEdge;
		}

		//Remove broken or deleted edges from the linked list.
		{
			EdgeType endEdge=island.mStartEdgeId;
			EdgeType undeletedEdge=island.mStartEdgeId;
			while(undeletedEdge!=INVALID_EDGE)
			{
				PX_ASSERT(allEdges[undeletedEdge].getIsConnected());

				EdgeType nextEdge=nextEdgeIds[undeletedEdge];
				while(nextEdge!=INVALID_EDGE && !allEdges[nextEdge].getIsConnected())
				{
					const EdgeType currEdge=nextEdge;
					nextEdge=nextEdgeIds[nextEdge];
					nextEdgeIds[currEdge]=INVALID_EDGE;
				}
				nextEdgeIds[undeletedEdge]=nextEdge;
				endEdge=undeletedEdge;
				undeletedEdge=nextEdge;
			}
			island.mEndEdgeId=endEdge;
		}

		PX_ASSERT(INVALID_EDGE==island.mStartEdgeId || island.mStartEdgeId!=nextEdgeIds[island.mStartEdgeId]);
	}
}

/**
\brief Edges that are broken or deleted are removed from their islands.
\param[in] brokenEdgeIds is an array of ids of all broken edges
\param[in] numBrokenEdges is the length of the array of broken edge ids (the number of broken edges).
\param[in] deletedEdgeIds is an array of ids of all deleted edges.
This will be NULL in 2nd pass because we can only delete edges prior to 1st pass.
\param[in] numDeletedEdges is the length of the array of broken edge ids (the number of broken edges).
This will be zero in 2nd pass because we can only delete edges prior to 1st pass.
\param[in] kinematicProxySourceNodes is a mapping between the id of a kinematic proxy node and the id of the source kinematic node.
This is null in first pass island gen but must be non-null in second pass island gen if there are any kinematics.
\param[in] nodeManager is array of all nodes used to look up nodes from affected edges.
\param[in,out] edgeManager is an array of all edges and a mapping that allows us to iterate over edges in an island given the first edge in the island.
Removing edges from islands changes the mapping.
\param[in,out]  islands is an array of all islands and a bitmap of active islands
Removing an edge can change the start edge of an island.
\param[out] brokenEdgeIslandsBitmap is a bitmap of all islands affected by a broken edge.
brokenEdgeIslandsBitmap should be cleared before callling this function.
\param[out] nodeStateChangeBitmap is a bitmap of all nodes affected by a self-state change or by changes to edge states.
Accumulate nodes affected by edge state changes in this function.
In 2nd pass island gen this is null because we have a simpler method of tracking affected nodes and islands.
*/
static void removeBrokenEdgesFromIslands
(const EdgeType* PX_RESTRICT brokenEdgeIds, const PxU32 numBrokenEdges, const EdgeType* PX_RESTRICT deletedEdgeIds, const PxU32 numDeletedEdges, 
 const NodeType* PX_RESTRICT kinematicProxySourceNodes, const NodeManager& nodeManager, 
 EdgeManager& edgeManager, IslandManager& islands,
 Cm::BitMap& brokenEdgeIslandsBitmap, Cm::BitMap* nodeStateChangeBitmap)
{
	const Node* PX_RESTRICT allNodes=nodeManager.getAll();
	Edge* PX_RESTRICT allEdges=edgeManager.getAll();

	//Gather a list of islands that contain a broken edge.
	for(PxU32 i=0;i<numBrokenEdges;i++)
	{
		//Get the edge that has just been broken and add it to the bitmap.
		const EdgeType edgeId=brokenEdgeIds[i];
		PX_ASSERT(edgeId<edgeManager.getCapacity());
		Edge& edge=allEdges[edgeId];

		//Check that the edge is legal.
		PX_ASSERT(!edge.getIsConnected());

		//Get the two nodes of the edge.
		//Get the two islands of the edge and add them to the bitmap.
		const NodeType nodeId1=edge.getNode1();
		const NodeType nodeId2=edge.getNode2();
		IslandType islandId1=INVALID_ISLAND;
		IslandType islandId2=INVALID_ISLAND;
		if(INVALID_NODE!=nodeId1)
		{
			PX_ASSERT(nodeId1<nodeManager.getCapacity());
			if(nodeStateChangeBitmap) nodeStateChangeBitmap->set(nodeId1);
			const Node& node1=allNodes[nodeId1];
			islandId1=node1.getIslandId();
			if(INVALID_ISLAND!=islandId1)
			{
				brokenEdgeIslandsBitmap.set(islandId1);
			}

			if (kinematicProxySourceNodes && node1.getIsKinematic())
			{
				//in the second island pass, we need to map the nodes of kinematics from the proxy
				//back to the original because the brokeness prevents them from being processed in 
				//with the other proxies.
				PX_ASSERT(kinematicProxySourceNodes[nodeId1] != INVALID_NODE);
				edge.setNode1(kinematicProxySourceNodes[nodeId1]);
			}
		}
		if(INVALID_NODE!=nodeId2)
		{
			PX_ASSERT(nodeId2<nodeManager.getCapacity());
			if(nodeStateChangeBitmap) nodeStateChangeBitmap->set(nodeId2);
			const Node& node2=allNodes[nodeId2];
			islandId2=node2.getIslandId();
			if(INVALID_ISLAND!=islandId2)
			{
				brokenEdgeIslandsBitmap.set(islandId2);
			}
			if (kinematicProxySourceNodes && node2.getIsKinematic())
			{
				//see comment in mirrored code above.
				PX_ASSERT(kinematicProxySourceNodes[nodeId2] != INVALID_NODE);
				edge.setNode2(kinematicProxySourceNodes[nodeId2]);
			}
		}
	}

	for(PxU32 i=0;i<numDeletedEdges;i++)
	{
		//Get the edge that has just been broken and add it to the bitmap.
		const EdgeType edgeId=deletedEdgeIds[i];
		PX_ASSERT(edgeId<edgeManager.getCapacity());
		Edge& edge=allEdges[edgeId];

		//If the edge was connected and was then deleted without having being disconnected
		//then we have to handle this as a broken edge.
		//The edge still has to be released so it can be reused.  This will be done at a later stage.
		if(edge.getIsConnected())
		{
			edge.setUnconnected();

			//Check that the edge is legal.
			PX_ASSERT(!edge.getIsConnected());

			//Get the two nodes of the edge.
			//Get the two islands of the edge and add them to the bitmap.
			const NodeType nodeId1=edge.getNode1();
			const NodeType nodeId2=edge.getNode2();
			IslandType islandId1=INVALID_ISLAND;
			IslandType islandId2=INVALID_ISLAND;
			if(INVALID_NODE!=nodeId1)
			{
				PX_ASSERT(nodeId1<nodeManager.getCapacity());
				if(nodeStateChangeBitmap) nodeStateChangeBitmap->set(nodeId1);
				islandId1=allNodes[nodeId1].getIslandId();
				if(INVALID_ISLAND!=islandId1)
				{
					brokenEdgeIslandsBitmap.set(islandId1);
				}
			}
			if(INVALID_NODE!=nodeId2)
			{
				PX_ASSERT(nodeId2<nodeManager.getCapacity());
				if(nodeStateChangeBitmap) nodeStateChangeBitmap->set(nodeId2);
				islandId2=allNodes[nodeId2].getIslandId();
				if(INVALID_ISLAND!=islandId2)
				{
					brokenEdgeIslandsBitmap.set(islandId2);
				}
			}
		}
	}

#define MAX_NUM_ISLANDS_TO_UPDATE 1024

	//Gather a simple list of all islands affected by broken edge or deleted node.
	IslandType islandsToUpdate[MAX_NUM_ISLANDS_TO_UPDATE];
	PxU32 numIslandsToUpdate=0;
	const PxU32 lastSetBit = brokenEdgeIslandsBitmap.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = brokenEdgeIslandsBitmap.getWords()[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
			PX_ASSERT(islandId!=INVALID_ISLAND);
			PX_ASSERT(islands.getBitmap().test(islandId));

			if(numIslandsToUpdate<MAX_NUM_ISLANDS_TO_UPDATE)
			{
				islandsToUpdate[numIslandsToUpdate]=islandId;
				numIslandsToUpdate++;
			}
			else
			{
				removeBrokenEdgesFromIslands2(islandsToUpdate,numIslandsToUpdate,edgeManager,islands);
				islandsToUpdate[0]=islandId;
				numIslandsToUpdate=1;
			}
		}
	}

	removeBrokenEdgesFromIslands2(islandsToUpdate,numIslandsToUpdate,edgeManager,islands);
}

/**
\brief Islands that are left empty as a consequence of deleted nodes are released so that they may be reused.
\param[in] emptyIslandsBitmap is a bitmap of all islands that are empty and need released.
\param[in] islands is a managed collection of islands.  This will be altered by releasing empty islands.
\param[in] brokenEdgeIslandsBitmap is a bitmap of islands affected by broken or deleted edges.  If an island is released 
then we no longer need to track this island in the bitmap.
*/
static void releaseEmptyIslands(const Cm::BitMap& emptyIslandsBitmap, IslandManager& islands, Cm::BitMap& brokenEdgeIslandsBitmap)
{
	const PxU32* PX_RESTRICT emptyIslandsBitmapWords=emptyIslandsBitmap.getWords();
	const PxU32 lastSetBit = emptyIslandsBitmap.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = emptyIslandsBitmapWords[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
			PX_ASSERT(islands.getBitmap().test(islandId));
			PX_ASSERT(INVALID_NODE==islands.get(islandId).mStartNodeId);
			PX_ASSERT(INVALID_NODE==islands.get(islandId).mEndNodeId);
			PX_ASSERT(INVALID_EDGE==islands.get(islandId).mStartEdgeId);
			PX_ASSERT(INVALID_EDGE==islands.get(islandId).mEndEdgeId);
			releaseIsland(islandId,islands);
			brokenEdgeIslandsBitmap.reset(islandId);
			PX_ASSERT(!islands.getBitmap().test(islandId));
		}
	}
}


/**
\brief Islands are merged as a consequence of edges that have been marked as joined.
\param[in] joinedEdges is an array of edge ids for all edges that have been marked as joined since last update
\param[in] numJoinedEdges is the number of edges that have been marked as joined since last update
\param nodeManager[in,out] is a managed collection of nodes.
Joined edges can merge two islands and update the node's island and the mapping that allows iteration over the nodes of an island.
\param edgeManager[in,out] is a managed collection of edges.
Joined edges can merge two islands and update the mapping that allows iteration over the edges of an island.
\param[in,out] brokenEdgeIslandsBitmap is a bitmap of all islands affected by a broken edge.  When two islands are merged we
discard one of the islands and need to update brokenEdgeIslandsBitmap accordingly.
\param graphNextNodes is a scratch buffer for accelerating island merging
\param graphStartIslands is a scratch buffer for accelerating island merging
\param graphNextIslands is a scratch buffer for accelerating island merging
\param[out] nodeStateChangeBitmap is a bitmap of all nodes affected by a self-state change or by changes to edge states.
Accumulate nodes affected by edge state changes in this function.
*/
static void processJoinedEdges
	(const EdgeType* PX_RESTRICT joinedEdges, const PxU32 numJoinedEdges, 
	NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
	Cm::BitMap& brokenEdgeIslandsBitmap, 
	Cm::BitMap& affectedIslandsBitmap,
	NodeType* PX_RESTRICT graphNextNodes, IslandType* PX_RESTRICT graphStartIslands, IslandType* PX_RESTRICT graphNextIslands,
	Cm::BitMap& nodeStateChangeBitmap)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	const PxU32 allNodesCapacity=nodeManager.getCapacity();
	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	const PxU32 allEdgesCapacity=edgeManager.getCapacity();

	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();

	PxMemSet(graphNextNodes, 0xff, sizeof(IslandType)*islands.getCapacity());
	PxMemSet(graphStartIslands, 0xff, sizeof(IslandType)*islands.getCapacity());
	PxMemSet(graphNextIslands, 0xff, sizeof(IslandType)*islands.getCapacity());

	//Add new nodes in a joined edge to an island on their own.
	//Record the bitmap of islands with nodes affected by joined edges.
	for(PxU32 i=0;i<numJoinedEdges;i++)
	{
		//Get the edge that has just been connected.
		const EdgeType edgeId=joinedEdges[i];
		PX_ASSERT(edgeId<allEdgesCapacity);
		const Edge& edge=allEdges[edgeId];

		if(!edge.getIsRemoved())
		{
			PX_ASSERT(edge.getIsConnected());

			const NodeType nodeId1=edge.getNode1();
			if(INVALID_NODE!=nodeId1)
			{
				nodeStateChangeBitmap.set(nodeId1);
				Node&  node=allNodes[nodeId1];
				IslandType islandId=node.getIslandId();
				if(INVALID_ISLAND!=islandId)
				{
					affectedIslandsBitmap.set(islandId);
				}
				else
				{
					PX_ASSERT(node.getIsNew());
					islandId=getNewIsland(islands);
					addNodeToIsland(islandId,nodeId1,allNodes,nextNodeIds,allNodesCapacity,islands);
					affectedIslandsBitmap.set(islandId);
				}
			}

			const NodeType nodeId2=edge.getNode2();
			if(INVALID_NODE!=nodeId2)
			{
				nodeStateChangeBitmap.set(nodeId2);
				Node&  node=allNodes[nodeId2];
				IslandType islandId=node.getIslandId();
				if(INVALID_ISLAND!=islandId)
				{
					affectedIslandsBitmap.set(islandId);
				}
				else
				{
					PX_ASSERT(node.getIsNew());
					islandId=getNewIsland(islands);
					addNodeToIsland(islandId,nodeId2,allNodes,nextNodeIds,allNodesCapacity,islands);
					affectedIslandsBitmap.set(islandId);
				}
			}
		}
	}

	//Iterate over all islands affected by a node in a joined edge.
	//Record all affected nodes and the start island of all affected nodes. 
	NodeType startNode=INVALID_NODE;
	IslandType prevIslandId=INVALID_ISLAND;
	const PxU32 lastSetBit = affectedIslandsBitmap.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = affectedIslandsBitmap.getWords()[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
			PX_ASSERT(islandId<islands.getCapacity());
			PX_ASSERT(islands.getBitmap().test(islandId));
			const Island& island=islands.get(islandId);

			NodeType nextNode=island.mStartNodeId;
			PX_ASSERT(INVALID_NODE!=nextNode);
			if(INVALID_NODE!=prevIslandId)
			{
				const Island& prevIsland=islands.get(prevIslandId);
				graphNextNodes[prevIsland.mEndNodeId]=island.mStartNodeId;
			}
			else
			{
				startNode=nextNode;
			}
			prevIslandId=islandId;

			while(INVALID_NODE!=nextNode)
			{
				const Node& node=allNodes[nextNode];
				const IslandType islandId1=node.getIslandId();
				PX_ASSERT(islandId1!=INVALID_ISLAND);
				graphNextNodes[nextNode]=nextNodeIds[nextNode];
				graphStartIslands[nextNode]=islandId1;
				graphNextIslands[islandId1]=INVALID_ISLAND;
				nextNode=nextNodeIds[nextNode];
			}
		}
	}

	//Join all the edges by merging islands.
	for(PxU32 i=0;i<numJoinedEdges;i++)
	{
		//Get the edge that has just been connected.
		const EdgeType edgeId=joinedEdges[i];
		PX_ASSERT(edgeId<allEdgesCapacity);
		const Edge& edge=allEdges[edgeId];

		if(!edge.getIsRemoved())
		{
			PX_ASSERT(edge.getIsConnected());

			//Get the two islands of the edge nodes.

			IslandType islandId1=INVALID_ISLAND;
			{
				const NodeType nodeId1=edge.getNode1();
				if(INVALID_NODE!=nodeId1)
				{
					PX_ASSERT(nodeId1<allNodesCapacity);
					IslandType nextIsland=graphStartIslands[nodeId1];
					while(INVALID_ISLAND!=nextIsland)
					{
						islandId1=nextIsland;
						nextIsland=graphNextIslands[nextIsland];
					}
				}
			}

			IslandType islandId2=INVALID_ISLAND;
			{
				const NodeType nodeId2=edge.getNode2();
				if(INVALID_NODE!=nodeId2)
				{
					PX_ASSERT(nodeId2<allNodesCapacity);
					IslandType nextIsland=graphStartIslands[nodeId2];
					while(INVALID_ISLAND!=nextIsland)
					{
						islandId2=nextIsland;
						nextIsland=graphNextIslands[nextIsland];
					}
				}
			}

			if(INVALID_ISLAND!=islandId1 && INVALID_ISLAND!=islandId2)
			{
				addEdgeToIsland(islandId1,edgeId,nextEdgeIds,allEdgesCapacity,islands);
				if(islandId1!=islandId2)
				{
					graphNextIslands[islandId2]=islandId1;

					PX_ASSERT(islands.getBitmap().test(islandId1));
					PX_ASSERT(islands.getBitmap().test(islandId2));
					joinIslands(islandId1,islandId2,allNodes,allNodesCapacity,allEdges,allEdgesCapacity,nextNodeIds,nextEdgeIds,islands);
					PX_ASSERT(islands.getBitmap().test(islandId1));
					PX_ASSERT(!islands.getBitmap().test(islandId2));
					if(brokenEdgeIslandsBitmap.test(islandId2))
					{
						brokenEdgeIslandsBitmap.set(islandId1);
						brokenEdgeIslandsBitmap.reset(islandId2);
					}
				}
			}
			else if(INVALID_ISLAND==islandId1 && INVALID_ISLAND!=islandId2)
			{
				addEdgeToIsland(islandId2,edgeId,nextEdgeIds,allEdgesCapacity,islands);
			}
			else if(INVALID_ISLAND!=islandId1 && INVALID_ISLAND==islandId2)
			{
				addEdgeToIsland(islandId1,edgeId,nextEdgeIds,allEdgesCapacity,islands);
			}
			else
			{
				PX_ASSERT(false);
			}
		}
	}

	//Set the island of all nodes in an island affected by a joined edge.
	NodeType nextNode=startNode;
	while(INVALID_NODE!=nextNode)
	{
		//startIslandId has changed to islandId.
		IslandType islandId=INVALID_ISLAND;
		IslandType nextIsland=graphStartIslands[nextNode];
		while(INVALID_ISLAND!=nextIsland)
		{
			islandId=nextIsland;
			nextIsland=graphNextIslands[nextIsland];
		}

		//Set the id of the node.
		Node& node=allNodes[nextNode];
		node.setIslandId(islandId);

		//Next node
		nextNode=graphNextNodes[nextNode];
	}
}

PX_FORCE_INLINE PxU32 alignSize16(const PxU32 size)
{
	return ((size + 15) & ~15);
}

#ifdef PX_DEBUG
NodeType getLastNode(NodeType* nextNodes, const NodeType nodeId)
{
	NodeType mapLast = nodeId;
	NodeType mapNext = nextNodes[nodeId];
	while (mapNext != INVALID_NODE)
	{
		mapLast = mapNext;
		mapNext = nextNodes[mapNext];
	}
	return mapLast;
}
#endif

/**
\brief Kinematic nodes must not act as bridges between islands. This is enforced by creating a proxy node each time an
edge references a node and replacing the edge's reference to the kinematic node with a reference to the proxy.
\param[in] kinematicNodesBitmap is a bitmap of all nodes that are kinematic
\param[in] processIslandsBitmap is a bitmap of all islands that need attention because a node or edge has changed state.
\param[in,out] nodeManager is a managed collection of nodes. 
We are going to create one new node each time an edge references a kinematic node. 
\param[in,out] edgeManager is a managed collection of edges
Edges that reference kinematic nodes will be modified so that they reference a kinematic proxy instead.
\param[in,out] islands is a managed collection of islands.
Islands that reference a kinematic node will have the kinematic node removed. 
\param[out] kinematicProxySourceNodes is a mapping that allows us to determine the id of the source kinematic node
given the id of one of its proxies.
\param[out] kinematicProxyNextNodes allows us to iterate over the list of all proxies for a given kinematic source node.
\param[out] kinematicProxyLastNodes allows us to determine the last proxy in the list of all proxies for a given kinematic source node.
\param[out] kinematicNodeIslandsBitmap is a bitmap of islands affected by kinematic nodes.  These islands have effectively experienced 
a broken edge event.
*/
static void duplicateKinematicNodes
(const Cm::BitMap& kinematicNodesBitmap, const Cm::BitMap& processIslandsBitmap,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
 NodeType* kinematicProxySourceNodes, NodeType* kinematicProxyNextNodes,  NodeType* kinematicProxyLastNodes,
 Cm::BitMap& kinematicNodeIslandsBitmap, Cm::BitMap& scratchBitmap)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	const PxU32 allNodesCapacity=nodeManager.getCapacity();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();

	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();

	PxMemSet(kinematicProxySourceNodes, 0xff, sizeof(NodeType)*allNodesCapacity);
	PxMemSet(kinematicProxyNextNodes, 0xff, sizeof(NodeType)*allNodesCapacity);
	PxMemSet(kinematicProxyLastNodes, 0xff, sizeof(NodeType)*allNodesCapacity);

	//Compute all the islands that need updated.
	//Set all kinematic nodes as deleted so they can be identified as needing removed from their islands.
	{
		const PxU32* PX_RESTRICT kinematicNodesBitmapWords=kinematicNodesBitmap.getWords();
		const PxU32 lastSetBit = kinematicNodesBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodesBitmapWords[w]; b; b &= b-1)
			{
				const NodeType kinematicNodeId = (NodeType)(w<<5|Ps::lowestSetBit(b));
				PX_ASSERT(kinematicNodeId<allNodesCapacity);
				Node& node=allNodes[kinematicNodeId];
				PX_ASSERT(node.getIsKinematic());

				//Get the island of the kinematic node and mark it as needing updated.
				const IslandType islandId=node.getIslandId();
				PX_ASSERT(INVALID_ISLAND!=islandId);
				PX_ASSERT(islands.getBitmap().test(islandId));
				if(processIslandsBitmap.test(islandId))
				{
					//We are interested in this island.
					kinematicNodeIslandsBitmap.set(islandId);

					//Set the node as deleted so it can be identified as needing to be removed.
					node.setIsDeleted();

					//Keep a track of kinematic nodes that were in islands to process.
					scratchBitmap.set(kinematicNodeId);
				}
			}
		}
	}

	//Remove all kinematics nodes from the islands.
	//Replace kinematics with proxies from each edge referencing each kinematic.
	{
		const PxU32 lastSetBit = kinematicNodeIslandsBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodeIslandsBitmap.getWords()[w]; b; b &= b-1)
			{
				const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				PX_ASSERT(islandId<islands.getCapacity());

				//Get the island.
				Island& island=islands.get(islandId);

				//Remove all kinematics from the island so that we can replace them with proxy nodes.
				removeDeletedNodesFromIsland(islandId,nodeManager,islands);
				
				//Create a proxy kinematic node for each edge that references a kinematic.
				EdgeType nextEdge=island.mStartEdgeId;
				while(nextEdge!=INVALID_EDGE)
				{
					//Get the current edge.
					PX_ASSERT(nextEdge<edgeManager.getCapacity());
					Edge& edge=allEdges[nextEdge];

					//Check the edge is legal.
					PX_ASSERT(edge.getIsConnected());
					PX_ASSERT(!edge.getIsRemoved());
					PX_ASSERT(edge.getNode1()!=INVALID_NODE || edge.getNode2()!=INVALID_NODE);

					//Add a proxy node for node1 if it is a kinematic.
					const NodeType nodeId1=edge.getNode1();
					if(INVALID_NODE!=nodeId1)
					{
						PX_ASSERT(nodeId1<allNodesCapacity);
						Node& node1=allNodes[nodeId1];
						if(node1.getIsKinematic())
						{
							PX_ASSERT(scratchBitmap.test(nodeId1));

							//Add a proxy node.
							const NodeType proxyNodeId=nodeManager.getAvailableElemNoResize();
							kinematicProxySourceNodes[proxyNodeId]=nodeId1;

							//Add it to the list of proxies for the source kinematic.
							NodeType mapLast = nodeId1;
							if(INVALID_NODE != kinematicProxyLastNodes[nodeId1])
							{
								mapLast = kinematicProxyLastNodes[nodeId1];
							}
							PX_ASSERT(getLastNode(kinematicProxyNextNodes, nodeId1) == mapLast);
							kinematicProxyNextNodes[mapLast]=proxyNodeId;
							kinematicProxyNextNodes[proxyNodeId]=INVALID_NODE;
							kinematicProxyLastNodes[nodeId1] = proxyNodeId;
							PX_ASSERT(getLastNode(kinematicProxyNextNodes, nodeId1) == kinematicProxyLastNodes[nodeId1]);

							//Set up the proxy node.
							//Don't copy the deleted flag across because that was artificially added to help remove the node from the island.
							Node& proxyNode=allNodes[proxyNodeId];
							proxyNode.setRigidBodyOwner(node1.getRigidBodyOwner());
							PX_ASSERT(node1.getIsDeleted());
							proxyNode.setFlags(PxU8(node1.getFlags() & ~Node::eDELETED));
							PX_ASSERT(!proxyNode.getIsDeleted());

							//Add the proxy to the island.
							addNodeToIsland(islandId,proxyNodeId,allNodes,nextNodeIds,allNodesCapacity,islands);
							PX_ASSERT(proxyNode.getIslandId()==islandId);

							//Set the edge to reference the proxy.
							edge.setNode1(proxyNodeId);
						}
					}

					//Add a proxy node for node2 if it is a kinematic.
					const NodeType nodeId2=edge.getNode2();
					if(INVALID_NODE!=nodeId2)
					{
						PX_ASSERT(nodeId2<allNodesCapacity);
						Node& node2=allNodes[nodeId2];
						if(node2.getIsKinematic())
						{
							PX_ASSERT(scratchBitmap.test(nodeId2));

							//Add a proxy node.
							const NodeType proxyNodeId=nodeManager.getAvailableElemNoResize();
							kinematicProxySourceNodes[proxyNodeId]=nodeId2;

							//Add it to the list of proxies for the source kinematic.
							NodeType mapLast = nodeId2;
							if(INVALID_NODE != kinematicProxyLastNodes[nodeId2])
							{
								mapLast = kinematicProxyLastNodes[nodeId2];
							}
							PX_ASSERT(getLastNode(kinematicProxyNextNodes, nodeId2) == mapLast);
							kinematicProxyNextNodes[mapLast]=proxyNodeId;
							kinematicProxyNextNodes[proxyNodeId]=INVALID_NODE;
							kinematicProxyLastNodes[nodeId2] = proxyNodeId;
							PX_ASSERT(getLastNode(kinematicProxyNextNodes, nodeId2) == kinematicProxyLastNodes[nodeId2]);

							//Set up the proxy node.
							//Don't copy the deleted flag across because that was artificially added to help remove the node from the island.
							Node& proxyNode=allNodes[proxyNodeId];
							proxyNode.setRigidBodyOwner(node2.getRigidBodyOwner());
							PX_ASSERT(node2.getIsDeleted());
							proxyNode.setFlags(PxU8(node2.getFlags() & ~Node::eDELETED));
							PX_ASSERT(!proxyNode.getIsDeleted());

							//Add the proxy to the island.
							addNodeToIsland(islandId,proxyNodeId,allNodes,nextNodeIds,allNodesCapacity,islands);
							PX_ASSERT(proxyNode.getIslandId()==islandId);

							//Set the edge to reference the proxy.
							edge.setNode2(proxyNodeId);
						}
					}
					
					nextEdge=nextEdgeIds[nextEdge];
				}
			}
		}
	}

	//Kinematics were flagged as deleted to identify them as needing removed from the island
	//so we nos need to clear the kinematic flag.  Take care that if a kinematic was referenced by 
	//no edges then it will have been removed flagged as deleted and then removed above but not 
	//replaced by anything.  Just put lone kinematics back in their original island.
	{
		const PxU32* PX_RESTRICT kinematicNodesBitmapWords=scratchBitmap.getWords();
		const PxU32 lastSetBit = scratchBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodesBitmapWords[w]; b; b &= b-1)
			{
				//Remember that we were only interested in some nodes.  
				//Those nodes were flagged as deleted at the start of this function.
				const NodeType kinematicNodeId = (NodeType)(w<<5|Ps::lowestSetBit(b));
				PX_ASSERT(kinematicNodeId<allNodesCapacity);
				Node& node=allNodes[kinematicNodeId];
				if(node.getIsDeleted())
				{
					PX_ASSERT(node.getIsKinematic());
					PX_ASSERT(node.getIsDeleted());
					PX_ASSERT(node.getIslandId()!=INVALID_ISLAND);
					PX_ASSERT(node.getIslandId()<islands.getCapacity());
					node.clearIsDeleted();

					//If the kinematic has no edges referencing it put it back in its own island.
					//If the kinematic has edges referencing it makes sense to flag the kinematic as having been replaced with proxies.
					if(INVALID_NODE==kinematicProxyNextNodes[kinematicNodeId])
					{
						const IslandType islandId=node.getIslandId();
						PX_ASSERT(INVALID_ISLAND!=islandId);
						PX_ASSERT(processIslandsBitmap.test(islandId));
						PX_ASSERT(kinematicNodeIslandsBitmap.test(islandId));
						addNodeToIsland(islandId,kinematicNodeId,allNodes,nextNodeIds,allNodesCapacity,islands);
					}
					else
					{
						//The node has been replaced with proxies so we can reset the node to neutral for now.
						node.setIslandId(INVALID_ISLAND);
					}
				}
			}
		}
	}
}

static void processBrokenEdgeIslands2
(const IslandType* islandsToUpdate, const PxU32 numIslandsToUpdate,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
 NodeType* graphNextNodes, IslandType* graphStartIslands, IslandType* graphNextIslands,
 Cm::BitMap& affectedIslandsBitmap)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	const PxU32 allNodesCapacity=nodeManager.getCapacity();
	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	const PxU32 allEdgesCapacity=edgeManager.getCapacity();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();

	PxMemSet(graphNextNodes, 0xff, sizeof(IslandType)*islands.getCapacity());
	PxMemSet(graphStartIslands, 0xff, sizeof(IslandType)*islands.getCapacity());
	PxMemSet(graphNextIslands, 0xff, sizeof(IslandType)*islands.getCapacity());

	for(PxU32 i=0;i<numIslandsToUpdate;i++)
	{
		//Get the island.
		const IslandType islandId0=islandsToUpdate[i];
		PX_ASSERT(islands.getBitmap().test(islandId0));
		const Island island0=islands.get(islandId0);
		const NodeType startNode=island0.mStartNodeId;
		const EdgeType startEdge=island0.mStartEdgeId;
		releaseIsland(islandId0,islands);

		//Create a dummy island for each node with a single node per island.
		NodeType nextNode=startNode;
		while(nextNode!=INVALID_NODE)
		{
			const IslandType newIslandId=getNewIsland(islands);
			graphNextNodes[nextNode]=nextNodeIds[nextNode];
			graphStartIslands[nextNode]=newIslandId;
			graphNextIslands[newIslandId]=INVALID_ISLAND;
			nextNode=nextNodeIds[nextNode];
		}

		//Add all the edges.
		EdgeType nextEdge=startEdge;
		while(nextEdge!=INVALID_EDGE)
		{
			const EdgeType currEdge=nextEdge;
			nextEdge=nextEdgeIds[nextEdge];

			//Get the current edge.
			PX_ASSERT(currEdge<allEdgesCapacity);
			Edge& edge=allEdges[currEdge];

			//Check the edge is legal.
			PX_ASSERT(edge.getIsConnected());
			PX_ASSERT(!edge.getIsRemoved());
			PX_ASSERT(edge.getNode1()!=INVALID_NODE || edge.getNode2()!=INVALID_NODE);

			//Get the two nodes of the edge.
			//Get the two islands of the edge.
			const NodeType nodeId1=edge.getNode1();
			const NodeType nodeId2=edge.getNode2();
			IslandType islandId1=INVALID_ISLAND;
			IslandType islandId2=INVALID_ISLAND;
			PxU32 depth1=0;
			PxU32 depth2=0;
			if(INVALID_NODE!=nodeId1)
			{
				PX_ASSERT(nodeId1<allNodesCapacity);
				IslandType nextIsland=graphStartIslands[nodeId1];
				PX_ASSERT(nextIsland!=INVALID_ISLAND);
				while(nextIsland!=INVALID_ISLAND)
				{
					islandId1=nextIsland;
					nextIsland=graphNextIslands[nextIsland];
					depth1++;
				}
			}
			if(INVALID_NODE!=nodeId2)
			{
				PX_ASSERT(nodeId2<allNodesCapacity);
				IslandType nextIsland=graphStartIslands[nodeId2];
				PX_ASSERT(nextIsland!=INVALID_ISLAND);
				while(nextIsland!=INVALID_ISLAND)
				{
					islandId2=nextIsland;
					nextIsland=graphNextIslands[nextIsland];
					depth2++;
				}
			}

			//Set island2 to be joined to island 1.
			if(INVALID_ISLAND!=islandId1 && INVALID_ISLAND!=islandId2)
			{
				if(islandId1!=islandId2)
				{
					if(depth1<depth2)
					{
						graphNextIslands[islandId1]=islandId2;
					}
					else
					{
						graphNextIslands[islandId2]=islandId1;
					}
				}
			}
			else if(INVALID_ISLAND==islandId1 && INVALID_ISLAND!=islandId2)
			{
				//Node2 is already in island 2.
				//Nothing to do.
			}
			else if(INVALID_ISLAND!=islandId1 && INVALID_ISLAND==islandId2)
			{
				//Node1 is already in island 1.
				//Nothing to do.
			}
			else
			{
				PX_ASSERT(false);
			}
		}

		//Go over all the nodes and add them to their islands.
		nextNode=startNode;
		while(nextNode!=INVALID_NODE)
		{
			IslandType islandId=graphStartIslands[nextNode];
			IslandType nextIsland=graphStartIslands[nextNode];
			PX_ASSERT(nextIsland!=INVALID_ISLAND);
			while(nextIsland!=INVALID_ISLAND)
			{
				islandId=nextIsland;
				nextIsland=graphNextIslands[nextIsland];
			}

			//Add the node to the island.
			//Island& island=islands.get(islandId);
			addNodeToIsland(islandId,nextNode,allNodes,nextNodeIds,allNodesCapacity,islands);

			//Next node.
			nextNode=graphNextNodes[nextNode];
		}

		//Release all empty islands.
		nextNode=startNode;
		while(nextNode!=INVALID_NODE)
		{
			const IslandType islandId=graphStartIslands[nextNode];
			Island& island=islands.get(islandId);
			if(INVALID_NODE==island.mStartNodeId)
			{
				PX_ASSERT(INVALID_NODE==island.mStartNodeId);
				PX_ASSERT(INVALID_NODE==island.mEndNodeId);
				PX_ASSERT(INVALID_EDGE==island.mStartEdgeId);
				PX_ASSERT(INVALID_EDGE==island.mEndEdgeId);
				releaseIsland(islandId,islands);

				affectedIslandsBitmap.reset(islandId);  // remove from list of islands to process
			}
			else
			{
				// If an island gets split up, we need to add the sub-islands to the list as well
				affectedIslandsBitmap.set(islandId);
			}

			//Next node.
			nextNode=graphNextNodes[nextNode];
		}

		//Now add all the edges to the islands.
		nextEdge=island0.mStartEdgeId;
		while(nextEdge!=INVALID_EDGE)
		{
			const EdgeType currEdge=nextEdge;
			nextEdge=nextEdgeIds[nextEdge];

			//Get the current edge.
			PX_ASSERT(currEdge<allEdgesCapacity);
			Edge& edge=allEdges[currEdge];

			//Check the edge is legal.
			PX_ASSERT(edge.getIsConnected());
			PX_ASSERT(!edge.getIsRemoved());
			PX_ASSERT(edge.getNode1()!=INVALID_NODE || edge.getNode2()!=INVALID_NODE);

			//Get the two nodes of the edge.
			//Get the two islands of the edge.
			const NodeType nodeId1=edge.getNode1();
			const NodeType nodeId2=edge.getNode2();
			PX_ASSERT(INVALID_NODE!=nodeId1 || INVALID_NODE!=nodeId2);
			if(INVALID_NODE!=nodeId1)
			{
				PX_ASSERT(nodeId1<allNodesCapacity);
				const IslandType islandId1=allNodes[nodeId1].getIslandId();
				PX_ASSERT(INVALID_ISLAND!=islandId1);
				PX_ASSERT(INVALID_NODE==nodeId2 || allNodes[nodeId2].getIslandId()==islandId1);
				addEdgeToIsland(islandId1,currEdge,nextEdgeIds,allEdgesCapacity,islands);
			}
			else if(INVALID_NODE!=nodeId2)
			{
				PX_ASSERT(nodeId2<allNodesCapacity);
				const IslandType islandId2=allNodes[nodeId2].getIslandId();
				PX_ASSERT(INVALID_ISLAND!=islandId2);
				PX_ASSERT(INVALID_NODE==nodeId1 || allNodes[nodeId1].getIslandId()==islandId2);
				addEdgeToIsland(islandId2,currEdge,nextEdgeIds,allEdgesCapacity,islands);
			}
		}
	}
}

/**
\brief All islands that have been marked as having a broken edge need to be rebuilt.  The result of this 
operation can range from generating the same island with the same node list to a separate island for each node.
\param[in] brokenEdgeIslandsBitmap is a bitmap of all islands marked as needing rebuilt.
\param[in] nodeManager is a managed collection of nodes.  
Rebuilding islands can result in nodes being in different islands.
\param[in] edgeManager is a managed collection of edges
Rebuilding islands can result in edges being in different islands.
\param[in] islands is a managed collection of islands.
Rebuilding islands can result in a different set of islands.
\param graphNextNodes is a scratch buffer that accelerates rebuilding islands.
\param graphStartIslands is a scratch buffer that accelerates rebuilding islands.
\param graphNextIslands is a scratch buffer that accelerates rebuilding islands.
\param[in,out] affectedIslandsBitmap is a bitmap of islands that have changed during 
the update. The function processBrokenEdgeIslands generates a different list of active island ids
so this needs reflected in affectedIslandsBitmap.
*/
static void processBrokenEdgeIslands
(const Cm::BitMap& brokenEdgeIslandsBitmap,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
 NodeType* graphNextNodes, IslandType* graphStartIslands, IslandType* graphNextIslands,
 Cm::BitMap& affectedIslandsBitmap)
{
#define MAX_NUM_ISLANDS_TO_UPDATE 1024

	//Gather a simple list of all islands affected by broken edge or deleted node.
	IslandType islandsToUpdate[MAX_NUM_ISLANDS_TO_UPDATE];
	PxU32 numIslandsToUpdate=0;
	const PxU32 lastSetBit = brokenEdgeIslandsBitmap.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = brokenEdgeIslandsBitmap.getWords()[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));

			if(islands.getBitmap().test(islandId))
			{
				if(numIslandsToUpdate<MAX_NUM_ISLANDS_TO_UPDATE)
				{
					islandsToUpdate[numIslandsToUpdate]=islandId;
					numIslandsToUpdate++;
				}
				else
				{
					processBrokenEdgeIslands2(islandsToUpdate,numIslandsToUpdate,nodeManager,edgeManager,islands,graphNextNodes,graphStartIslands,graphNextIslands,
												affectedIslandsBitmap);
					islandsToUpdate[0]=islandId;
					numIslandsToUpdate=1;
				}
			}
		}
	}

	processBrokenEdgeIslands2(islandsToUpdate,numIslandsToUpdate,nodeManager,edgeManager,islands,graphNextNodes,graphStartIslands,graphNextIslands,
								affectedIslandsBitmap);
}

/**
\brief All edges that are marked as deleted are prepared for re-use.
\param[in] deletedEdges is an array of ids of all deleted edges.
\param[in] numDeletedEdges is the number of edges that have been deleted.
\param[in] edgeManager is a managed collection of nodes.  
Edges that are deleted can be recycled.  
*/
static void releaseDeletedEdges(const EdgeType* PX_RESTRICT deletedEdges, const PxU32 numDeletedEdges, EdgeManager& edgeManager)
{
	//Now release the deleted edges.
	for(PxU32 i=0;i<numDeletedEdges;i++)
	{
		//Get the deleted edge.
		const EdgeType edgeId=deletedEdges[i];

		//Test the edge is legal.
		PX_ASSERT(edgeManager.get(edgeId).getIsRemoved());

		//Release the edge.
		edgeManager.release(edgeId);
	}
}

/**
\Brief All new nodes are given their own island.
\param[in] createdNodes is an array of ids of all nodes that have been freshly created.
\param[in] numCreatedNodes is the number of nodes that have been freshly created.
\param[in,out] nodeManager is a managed collection of nodes.
Nodes that are created are initially put in an island containing just the created node. The id of the island 
is stored on the node.
\param[in,out] islands is a managed collection of islands.
Nodes that are created are initially put in a new island containing just the created node. 
*/
static void processCreatedNodes
(const NodeType* PX_RESTRICT createdNodes, const PxU32 numCreatedNodes, NodeManager& nodeManager, IslandManager& islands)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	const PxU32 allNodesCapacity=nodeManager.getCapacity();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();

	for(PxU32 i=0;i<numCreatedNodes;i++)
	{
		const NodeType nodeId=createdNodes[i];
		PX_ASSERT(nodeId<allNodesCapacity);
		Node& node=allNodes[nodeId];
		node.clearIsNew();

		//If a body was added after PxScene.simulate and before PxScene.fetchResults then 
		//the addition will be delayed and not processed until the end of fetchResults.
		//If this body is then released straight after PxScene.fetchResults then at the 
		//next PxScene.simulate we will have a body that has been both added and removed.
		//The removal must take precedence over the addition.
		if(!node.getIsDeleted() && node.getIslandId()==INVALID_ISLAND)
		{
			const IslandType islandId=getNewIsland(islands);
			PX_ASSERT(islands.getCapacity()<=allNodesCapacity);
			addNodeToIsland(islandId,nodeId,allNodes,nextNodeIds,allNodesCapacity,islands);
		}
	}
}

/**
\brief All deleted nodes are prepared for re-used.
\param[in] deletedNodes is an array of ids of all deleted nodes.
\param[in] numDeletedNodes is the number of nodes that have been deleted.
\param[in] nodeManager is a managed collection of nodes.  
Nodes that are deleted can be recycled.  
\param[out] nodeStateChangeBitmap is a bitmap of all nodes affected by a self-state change or by changes to edge states.
Any node that has been deleted no longer needs tracked.
*/
static void releaseDeletedNodes(const NodeType* PX_RESTRICT deletedNodes, const PxU32 numDeletedNodes, NodeManager& nodeManager, Cm::BitMap& nodeStateChangeBitmap)
{
	for(PxU32 i=0;i<numDeletedNodes;i++)
	{
		const NodeType nodeId=deletedNodes[i];
		PX_ASSERT(nodeManager.get(nodeId).getIsDeleted());
		nodeManager.release(nodeId);
		nodeStateChangeBitmap.reset(nodeId);
	}
}	

static const bool tSecondPass = true;  // to make use of template parameter more readable

/**
\brief  The final list of islands is parsed to determine which islands are asleep and which are awake so that sleep state changes may be reported 
and nodes/edges in awake islands can be sent to the solver.  
\param[in] islandsToUpdate is a bitmap of all islands that need parsed.
\param[in] rigidBodyOffset is a byte offset between the body ptr stored by a node and the body ptr reported to the solver.
\param[in,out] nodeManager is a managed collection of nodes
Nodes may be flagged as being in sleeping in islands or as being in awake islands.
\param[in] edgeManager is a managed collection of edges
\param[in] islands is managed collection of islands
\param[in] articulationRootManager is a managed collection of articulation roots
\param[in] kinematicSourceNodeIds allows us to get the source kinematic node from any of its proxies.
\param[out] psicData is a structure of data that we gather and report to the solver
*/
#ifndef __SPU__
template <bool TSecondPass>
static void processSleepingIslands
(const Cm::BitMap& islandsToUpdate, const PxU32 rigidBodyOffset, 
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager, 
 const NodeType* kinematicSourceNodeIds,
 ProcessSleepingIslandsComputeData& psicData)
{
	PX_UNUSED(kinematicSourceNodeIds);

	if (!TSecondPass)
	{
		psicData.mBodiesToWakeSize=0;
		psicData.mBodiesToSleepSize=0;
		psicData.mNarrowPhaseContactManagersSize=0;
		psicData.mSolverKinematicsSize=0;
		psicData.mSolverBodiesSize=0;
		psicData.mSolverArticulationsSize=0;
		psicData.mSolverContactManagersSize=0;
		psicData.mSolverConstraintsSize=0;
		psicData.mIslandIndicesSize=0;
		psicData.mIslandIndicesSecondPassSize=0;
	}
	else
	{
		psicData.mBodiesToWakeSize=0;
		psicData.mBodiesToSleepSize=0;
	}

	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();
	//const PxU32 allNodesCapacity=nodeManager.getCapacity();
	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();
	//const PxU32 allEdgesCapacity=edgeManager.getCapacity();
	ArticulationRoot* PX_RESTRICT allArticRoots=articulationRootManager.getAll();
	//const PxU32 allArtisCootsCapacity=articulationRootManager.getCapacity();
	Island* PX_RESTRICT allIslands=islands.getAll();
	//const PxU32 allIslandsCapacity=islands.getCapacity();

	NodeType* PX_RESTRICT solverBodyMap=psicData.mSolverBodyMap;
	//const PxU32 solverBodyMapCapacity=psicData.mSolverBodyMapCapacity;
	PxU8** PX_RESTRICT bodiesToWakeOrSleep=psicData.mBodiesToWakeOrSleep;
	const PxU32 bodiesToWakeOrSleepCapacity=psicData.mBodiesToWakeOrSleepCapacity;
	NarrowPhaseContactManager* PX_RESTRICT npContactManagers=psicData.mNarrowPhaseContactManagers;
	if (TSecondPass)
		PX_UNUSED(npContactManagers);
	//const PxU32 npContactManagersCapacity=psicData.mNarrowPhaseContactManagersCapacity;
	PxsRigidBody** PX_RESTRICT solverKinematics=psicData.mSolverKinematics;
	//const PxU32 solverKinematicsCapacity=psicData.mSolverKinematicsCapacity;
	PxsRigidBody** PX_RESTRICT solverBodies=psicData.mSolverBodies;
	//const PxU32 solverBodiesCapacity=psicData.mSolverBodiesCapacity;
	PxsArticulation** PX_RESTRICT solverArticulations=psicData.mSolverArticulations;
	void** PX_RESTRICT solverArticulationOwners=psicData.mSolverArticulationOwners;
	//const PxU32 solverArticulationsCapacity=psicData.mSolverArticulationsCapacity;
	PxsIndexedContactManager* PX_RESTRICT solverContactManagers=psicData.mSolverContactManagers;
	//const PxU32 solverContactManagersCapacity=psicData.mSolverContactManagersCapacity;
	PxsIndexedConstraint* PX_RESTRICT solverConstraints=psicData.mSolverConstraints;
	//const PxU32 solverConstraintsCapacity=psicData.mSolverConstraintsCapacity;
	PxsIslandIndices* PX_RESTRICT islandIndices=psicData.mIslandIndices;
	const PxU32 islandIndicesCapacity=psicData.mIslandIndicesCapacity;

	PxU32 bodiesToWakeSize=0;
	if (TSecondPass)
		PX_UNUSED(bodiesToWakeSize);
	PxU32 bodiesToSleepSize=0;
	PxU32 npContactManagersSize=0;
	if (TSecondPass)
		PX_UNUSED(npContactManagersSize);
	PxU32 solverKinematicsSize=psicData.mSolverKinematicsSize;
	PxU32 solverBodiesSize=psicData.mSolverBodiesSize;
	PxU32 solverArticulationsSize=psicData.mSolverArticulationsSize;
	PxU32 solverContactManagersSize=psicData.mSolverContactManagersSize;
	PxU32 solverConstraintsSize=psicData.mSolverConstraintsSize;
	PxU32 islandIndicesSize=psicData.mIslandIndicesSize;
	PxU32 islandIndicesSecondPassSize = psicData.mIslandIndicesSecondPassSize;
	if (TSecondPass)
		PX_UNUSED(islandIndicesSecondPassSize);

	const PxU32* PX_RESTRICT bitmapWords=islandsToUpdate.getWords();
	const PxU32 lastSetBit = islandsToUpdate.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = bitmapWords[w]; b; b &= b-1)
		{
			const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
	
			PX_ASSERT(islandId<islands.getCapacity());
			PX_ASSERT(islands.getBitmap().test(islandId));
			const Island& island=allIslands[islandId];

			//First compute the new state of the island 
			//(determine if any nodes have non-zero wake counter).
			NodeType nextNode0=island.mStartNodeId;
			PxU32 islandFlags=0;
			while(INVALID_NODE!=nextNode0)
			{
				PX_ASSERT(nextNode0<nodeManager.getCapacity());
				const Node& node=allNodes[nextNode0];
				islandFlags |= node.getFlags();
				nextNode0=nextNodeIds[nextNode0];
			}

			if(0 == (islandFlags & Node::eNOTREADYFORSLEEPING))
			{
				//Island is asleep because no nodes have non-zero wake counter.
				NodeType nextNode=island.mStartNodeId;
				while(nextNode!=INVALID_NODE)
				{
					//Get the node.
					PX_ASSERT(nextNode<nodeManager.getCapacity());
					Node& node=allNodes[nextNode];
					Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&allNodes[nextNodeIds[nextNode]]) + 128) & ~127));
					PX_ASSERT(node.getIsReadyForSleeping());
					// in second island gen pass all nodes should have been woken up.
					PX_ASSERT(!TSecondPass || node.getIsKinematic() || (node.getIsArticulated() && !node.getIsRootArticulationLink()) || !node.getIsInSleepingIsland());

					//Work out if the node was previously in a sleeping island.
					const bool wasInSleepingIsland=node.getIsInSleepingIsland();
					if (TSecondPass)
						PX_UNUSED(wasInSleepingIsland);

					//If the node has changed from not being in a sleeping island to 
					//being in a sleeping island then the node needs put to sleep
					//in the high level.  Store the body pointer for reporting to hl.
					if(TSecondPass || (!wasInSleepingIsland))
					{
						if(!node.getIsArticulated())
						{
							if(!node.getIsKinematic())
							{
								//Set the node to be in a sleeping island.
								node.setIsInSleepingIsland();

								PX_ASSERT((bodiesToWakeSize+bodiesToSleepSize)<bodiesToWakeOrSleepCapacity);
								PX_ASSERT(0==((size_t)node.getRigidBodyOwner() & 0x0f));
								bodiesToWakeOrSleep[bodiesToWakeOrSleepCapacity-1-bodiesToSleepSize]=(PxU8*)node.getRigidBodyOwner();
								Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&bodiesToWakeOrSleep[bodiesToWakeOrSleepCapacity-1-bodiesToSleepSize-1]) + 128) & ~127));
								bodiesToSleepSize++;
							}
						}
						else if(node.getIsRootArticulationLink())
						{
							//Set the node to be in a sleeping island.
							node.setIsInSleepingIsland();

							const PxU32 rootArticId=(PxU32)node.getRootArticulationId();
							const ArticulationRoot& rootArtic=allArticRoots[rootArticId];
							PxU8* articOwner=(PxU8*)rootArtic.mArticulationOwner;
							PX_ASSERT(0==((size_t)articOwner & 0x0f));
							PX_ASSERT((bodiesToWakeSize+bodiesToSleepSize)<bodiesToWakeOrSleepCapacity);
							bodiesToWakeOrSleep[bodiesToWakeOrSleepCapacity-1-bodiesToSleepSize]=(PxU8*)((size_t)articOwner | 0x01);
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&bodiesToWakeOrSleep[bodiesToWakeOrSleepCapacity-1-bodiesToSleepSize-1]) + 128) & ~127));
							bodiesToSleepSize++;
						}
					}

					nextNode=nextNodeIds[nextNode];
				}
			}
			else
			{
				//Island is awake because at least one node has a non-zero wake counter.
				PX_ASSERT(island.mStartNodeId!=INVALID_NODE);

				//If an island needs a second pass, we need to revert the counters of written out data
				const PxU32 previousSolverKinematicsSize=solverKinematicsSize;
				const PxU32 previousSolverBodiesSize=solverBodiesSize;
				const PxU32 previousSolverArticulationsSize=solverArticulationsSize;
				const PxU32 previousSolverContactManagersSize=solverContactManagersSize;
				const PxU32 previousSolverConstraintsSize=solverConstraintsSize;

				//Add the indices of the island.
				PX_ASSERT(TSecondPass || (islandIndicesSize<(islandIndicesCapacity - islandIndicesSecondPassSize)));
				islandIndices[islandIndicesSize].articulations=solverArticulationsSize;
				islandIndices[islandIndicesSize].bodies=(NodeType)solverBodiesSize;
				islandIndices[islandIndicesSize].contactManagers=(EdgeType)solverContactManagersSize;
				islandIndices[islandIndicesSize].constraints=(EdgeType)solverConstraintsSize;

				Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&islandIndices[islandIndicesSize+1]) + 128) & ~127));

				NodeType nextNode=island.mStartNodeId;
				while(nextNode!=INVALID_NODE)
				{
					//Get the node.
					PX_ASSERT(nextNode<nodeManager.getCapacity());
					Node& node=allNodes[nextNode];
					Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&allNodes[nextNodeIds[nextNode]]) + 128) & ~127));

					//Work out if the node was previously in a sleeping island.
					const bool wasInSleepingIsland=node.getIsInSleepingIsland();
					if (TSecondPass)
						PX_UNUSED(wasInSleepingIsland);

					// in second island gen pass all nodes should have been woken up.
					PX_ASSERT(!TSecondPass || node.getIsKinematic() || (node.getIsArticulated() && !node.getIsRootArticulationLink()) || !node.getIsInSleepingIsland());

					//If the node has changed from being in a sleeping island to 
					//not being in a sleeping island then the node needs to be woken up
					//in the high level.  Store the body pointer for reporting to hl.
					if ((!TSecondPass) && wasInSleepingIsland)
					{
						if(!node.getIsArticulated())
						{
							if(!node.getIsKinematic())
							{
								node.clearIsInSleepingIsland();

								PX_ASSERT((bodiesToWakeSize+bodiesToSleepSize)<bodiesToWakeOrSleepCapacity);
								PX_ASSERT(0==((size_t)node.getRigidBodyOwner() & 0x0f));
								bodiesToWakeOrSleep[bodiesToWakeSize]=(PxU8*)node.getRigidBodyOwner();
								Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&bodiesToWakeOrSleep[bodiesToWakeSize+1]) + 128) & ~127));
								bodiesToWakeSize++;
							}
						}
						else if(node.getIsRootArticulationLink())
						{
							node.clearIsInSleepingIsland();

							PX_ASSERT((bodiesToWakeSize+bodiesToSleepSize)<bodiesToWakeOrSleepCapacity);
							const PxU32 rootArticId=(PxU32)node.getRootArticulationId();
							const ArticulationRoot& rootArtic=allArticRoots[rootArticId];
							PxU8* articOwner=(PxU8*)rootArtic.mArticulationOwner;
							PX_ASSERT(0==((size_t)articOwner & 0x0f));
							bodiesToWakeOrSleep[bodiesToWakeSize]=(PxU8*)((size_t)articOwner | 0x01);
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&bodiesToWakeOrSleep[bodiesToWakeSize+1]) + 128) & ~127));
							bodiesToWakeSize++;
						}
					}

					if(!node.getIsKinematic())
					{
						if(!node.getIsArticulated())
						{
							//Create the mapping between the entry id in mNodeManager and the entry id in mSolverBoldies
							PX_ASSERT(nextNode<psicData.mSolverBodyMapCapacity);
							solverBodyMap[nextNode]=(NodeType)solverBodiesSize;

							//Add rigid body to solver island.
							PX_ASSERT(solverBodiesSize<psicData.mSolverBodiesCapacity);
							solverBodies[solverBodiesSize]=node.getRigidBody(rigidBodyOffset);
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverBodies[solverBodiesSize+1]) + 128) & ~127));
							solverBodiesSize++;
						}
						else if(node.getIsRootArticulationLink())
						{
							//Add articulation to solver island.
							const PxU32 rootArticId=(PxU32)node.getRootArticulationId();
							const ArticulationRoot& rootArtic=allArticRoots[rootArticId];
							PxsArticulationLinkHandle articLinkHandle=rootArtic.mArticulationLinkHandle;
							void* articOwner=rootArtic.mArticulationOwner;
							PX_ASSERT((solverArticulationsSize)<psicData.mSolverArticulationsCapacity);
							solverArticulations[solverArticulationsSize]=getArticulation(articLinkHandle);
							solverArticulationOwners[solverArticulationsSize]=articOwner;
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverArticulations[solverArticulationsSize+1]) + 128) & ~127));
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverArticulationOwners[solverArticulationsSize+1]) + 128) & ~127));
							solverArticulationsSize++;
						}
					}
					else
					{
						//Create the mapping between the entry id in mNodeManager and the entry id in mSolverBoldies
						PX_ASSERT(nextNode<psicData.mSolverBodyMapCapacity);
						solverBodyMap[nextNode]=(NodeType)solverKinematicsSize;

						//Add kinematic to array of all kinematics.
						PX_ASSERT(!node.getIsArticulated());
						PX_ASSERT((solverKinematicsSize)<psicData.mSolverKinematicsCapacity);
						solverKinematics[solverKinematicsSize]=node.getRigidBody(rigidBodyOffset);
						Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverKinematics[solverKinematicsSize+1]) + 128) & ~127));
						solverKinematicsSize++;
					}
	
					nextNode=nextNodeIds[nextNode];
				}

				bool hasStaticContact=false;
				bool hasSecondPassPairs=false;  // island has pairs that need a second narrowphase/island gen pass
				EdgeType nextEdgeId=island.mStartEdgeId;
				while(nextEdgeId!=INVALID_EDGE)
				{
					PX_ASSERT(nextEdgeId<edgeManager.getCapacity());
					Edge& edge=allEdges[nextEdgeId];
					Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&allEdges[nextEdgeIds[nextEdgeId]]) + 128) & ~127));

					const NodeType node1Id=edge.getNode1();
					const NodeType node2Id=edge.getNode2();
					PxU8 node1Type=PxsIndexedInteraction::eWORLD;
					PxU8 node2Type=PxsIndexedInteraction::eWORLD;
					PxsArticulationLinkHandle body1=(PxsArticulationLinkHandle)-1;
					PxsArticulationLinkHandle body2=(PxsArticulationLinkHandle)-1;
					bool node1IsKineOrStatic=true;
					bool node2IsKineOrStatic=true;

					if (TSecondPass || (!hasSecondPassPairs))  //we can skip a lot of logic as soon as we know the island needs to go through a second pass
					{
						if(node1Id!=INVALID_NODE)
						{
							PX_ASSERT(node1Id<nodeManager.getCapacity());
							const Node& node1=allNodes[node1Id];
							node1IsKineOrStatic=false;
							if(!node1.getIsArticulated())
							{
								node1IsKineOrStatic=node1.getIsKinematic();
								node1Type=(!node1IsKineOrStatic) ? (PxU8)PxsIndexedInteraction::eBODY : (PxU8)PxsIndexedInteraction::eKINEMATIC;
								PX_ASSERT(node1Id<psicData.mSolverBodyMapCapacity);
								body1=solverBodyMap[node1Id];
								PX_ASSERT((node1.getIsKinematic() && body1<psicData.mSolverKinematicsCapacity) || (!node1.getIsKinematic() && body1<psicData.mSolverBodiesCapacity));
							}
							else if(!node1.getIsRootArticulationLink())
							{
								node1Type=PxsIndexedInteraction::eARTICULATION;
								body1=node1.getArticulationLink();
							}
							else
							{
								node1Type=PxsIndexedInteraction::eARTICULATION;
								const PxU32 articRootId=(PxU32)node1.getRootArticulationId();
								const ArticulationRoot& articRoot=allArticRoots[articRootId];
								body1=articRoot.mArticulationLinkHandle;
							}
						}
						else
						{
							hasStaticContact=true;
						}
						if(node2Id!=INVALID_NODE)
						{
							PX_ASSERT(node2Id<nodeManager.getCapacity());
							const Node& node2=allNodes[node2Id];
							node2IsKineOrStatic=false;
							if(!node2.getIsArticulated())
							{
								node2IsKineOrStatic=node2.getIsKinematic();
								node2Type=(!node2IsKineOrStatic) ? (PxU8)PxsIndexedInteraction::eBODY : (PxU8)PxsIndexedInteraction::eKINEMATIC;
								PX_ASSERT(node2Id<psicData.mSolverBodyMapCapacity);
								body2=solverBodyMap[node2Id];
								PX_ASSERT((node2.getIsKinematic() && body2<psicData.mSolverKinematicsCapacity) || (!node2.getIsKinematic() && body2<psicData.mSolverBodiesCapacity));
							}
							else if(!node2.getIsRootArticulationLink())
							{
								node2Type=PxsIndexedInteraction::eARTICULATION;
								body2=node2.getArticulationLink();
							}						
							else
							{
								node2Type=PxsIndexedInteraction::eARTICULATION;
								const PxU32 articRootId=(PxU32)node2.getRootArticulationId();
								const ArticulationRoot& articRoot=allArticRoots[articRootId];
								body2=articRoot.mArticulationLinkHandle;
							}
						}
						else
						{
							hasStaticContact=true;
						}
					}
					else
					{
						if(node1Id!=INVALID_NODE)
						{
							PX_ASSERT(node1Id<nodeManager.getCapacity());
							const Node& node1=allNodes[node1Id];
							node1IsKineOrStatic=false;
							if(!node1.getIsArticulated())
							{
								node1IsKineOrStatic=node1.getIsKinematic();
							}
						}
						
						if(node2Id!=INVALID_NODE)
						{
							PX_ASSERT(node2Id<nodeManager.getCapacity());
							const Node& node2=allNodes[node2Id];
							node2IsKineOrStatic=false;
							if(!node2.getIsArticulated())
							{
								node2IsKineOrStatic=node2.getIsKinematic();
							}
						}
					}

					//Work out if we have an interaction handled by the solver.
					//The solver doesn't handle kinematic-kinematic or kinematic-static
					const bool isSolverInteraction = (!node1IsKineOrStatic || !node2IsKineOrStatic);

					//If both nodes were asleep at the last island gen then they need to undergo a narrowphase overlap prior to running the solver.
					//It is possible that one or other of the nodes has been woken up since the last island gen.  If either was woken up in this way then 
					//the pair will already have a contact manager provided by high-level. This being the case, the narrowphase overlap of the pair 
					//will already have been performed prior to the current island gen.  If neither has been woken up since the last island gen then 
					//there will be no contact manager and narrowphase needs to be performed on the pair in a second pass after the island gen.  The bodies 
					//will be woken up straight after the current island gen (because they are in the array of bodies to be woken up) and the pair will be 
					//given a contact manager by high-level.
					if(!TSecondPass && edge.getIsTypeCM() && !edge.getCM() && isSolverInteraction)
					{
						//Add to list for second np pass.
						PX_ASSERT((npContactManagersSize)<psicData.mNarrowPhaseContactManagersCapacity);
						npContactManagers[npContactManagersSize].mEdgeId=nextEdgeId;
						npContactManagers[npContactManagersSize].mCM=NULL;
						Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&npContactManagers[npContactManagersSize+1]) + 128) & ~127));
						npContactManagersSize++;
						hasSecondPassPairs = true;
					}

					if (TSecondPass || (!hasSecondPassPairs))
					{
						if(edge.getIsTypeCM() && isSolverInteraction)
						{
							//Add to contact managers for the solver island.
							PX_ASSERT((solverContactManagersSize)<psicData.mSolverContactManagersCapacity);
							PxsIndexedContactManager& interaction=solverContactManagers[solverContactManagersSize];
							interaction.contactManager=edge.getCM();
							interaction.indexType0=node1Type;
							interaction.indexType1=node2Type;
							interaction.solverBody0=body1;
							interaction.solverBody1=body2;
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverContactManagers[solverContactManagersSize+1]) + 128) & ~127));
							solverContactManagersSize++;
						}
						else if(edge.getIsTypeConstraint() && isSolverInteraction)
						{
							//Add to constraints for the solver island.
							PX_ASSERT((solverConstraintsSize)<psicData.mSolverConstraintsCapacity);
							PxsIndexedConstraint& interaction=solverConstraints[solverConstraintsSize];
							interaction.constraint=edge.getConstraint();
							interaction.indexType0=node1Type;
							interaction.indexType1=node2Type;
							interaction.solverBody0=body1;
							interaction.solverBody1=body2;
							Ps::prefetchLine((PxU8*)(((size_t)((PxU8*)&solverConstraints[solverConstraintsSize+1]) + 128) & ~127));
							solverConstraintsSize++;
						}
						else
						{
							//Already stored in the articulation.
							PX_ASSERT(edge.getIsTypeArticulation() || !isSolverInteraction);
						}
					}

					nextEdgeId=nextEdgeIds[nextEdgeId];
				}

				if (TSecondPass || (!hasSecondPassPairs))
				{
					//Record if the island has static contact and increment the island size.
					islandIndices[islandIndicesSize].setHasStaticContact(hasStaticContact);
					islandIndicesSize++;
				}
				else
				{
					//The island needs a second pass (narrowphase for sure, maybe second island gen pass too) -> track the island.
					PX_ASSERT((islandIndicesSecondPassSize + islandIndicesSize) < islandIndicesCapacity);
					islandIndicesSecondPassSize++;
					islandIndices[islandIndicesCapacity - islandIndicesSecondPassSize].islandId=(IslandType)islandId;

					//Revert the stored solver information
					solverKinematicsSize = previousSolverKinematicsSize;
					solverBodiesSize = previousSolverBodiesSize;
					solverArticulationsSize = previousSolverArticulationsSize;
					solverContactManagersSize = previousSolverContactManagersSize;
					solverConstraintsSize = previousSolverConstraintsSize;
				}
			}
		}
	}

	// add end count item
	PX_ASSERT(islandIndicesSize<=islands.getCapacity());
	psicData.mIslandIndices[islandIndicesSize].bodies=(NodeType)solverBodiesSize;
	psicData.mIslandIndices[islandIndicesSize].articulations=(NodeType)solverArticulationsSize;
	psicData.mIslandIndices[islandIndicesSize].contactManagers=(EdgeType)solverContactManagersSize;
	psicData.mIslandIndices[islandIndicesSize].constraints=(EdgeType)solverConstraintsSize;

	if (!TSecondPass)
		psicData.mBodiesToWakeSize=bodiesToWakeSize;
	psicData.mBodiesToSleepSize=bodiesToSleepSize;
	if (!TSecondPass)
		psicData.mNarrowPhaseContactManagersSize=npContactManagersSize;
	psicData.mSolverKinematicsSize=solverKinematicsSize;
	psicData.mSolverBodiesSize=solverBodiesSize;
	psicData.mSolverArticulationsSize=solverArticulationsSize;
	psicData.mSolverContactManagersSize=solverContactManagersSize;
	psicData.mSolverConstraintsSize=solverConstraintsSize;
	psicData.mIslandIndicesSize=islandIndicesSize;
	if (TSecondPass)
		psicData.mIslandIndicesSecondPassSize=0;
	else
		psicData.mIslandIndicesSecondPassSize=islandIndicesSecondPassSize;
}
#endif

void physx::mergeKinematicProxiesBackToSource
(const Cm::BitMap& kinematicNodesBitmap,
 const NodeType* PX_RESTRICT kinematicProxySourceNodes, const NodeType* PX_RESTRICT kinematicProxyNextNodes,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands,
 Cm::BitMap& kinematicIslandsBitmap,
 IslandType* graphStartIslands, IslandType* graphNextIslands)
{
	Node* PX_RESTRICT allNodes=nodeManager.getAll();
	const PxU32 allNodesCapacity=nodeManager.getCapacity();
	NodeType* PX_RESTRICT nextNodeIds=nodeManager.getNextNodeIds();

	Edge* PX_RESTRICT allEdges=edgeManager.getAll();
	const PxU32 allEdgesCapacity=edgeManager.getCapacity();
	EdgeType* PX_RESTRICT nextEdgeIds=edgeManager.getNextEdgeIds();

	PxMemSet(graphStartIslands, 0xff, sizeof(IslandType)*islands.getCapacity());
	PxMemSet(graphNextIslands, 0xff, sizeof(IslandType)*islands.getCapacity());

	//Remove all the proxies from all islands.

	//Compute the bitmap of islands affected by kinematic proxy nodes.
	//Work out if the node was in a non-sleeping island.
	{
		const PxU32* PX_RESTRICT kinematicNodesBitmapWords=kinematicNodesBitmap.getWords();
		const PxU32 lastSetBit = kinematicNodesBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodesBitmapWords[w]; b; b &= b-1)
			{
				const NodeType kinematicNodeId = (NodeType)(w<<5|Ps::lowestSetBit(b));
				PX_ASSERT(kinematicNodeId<allNodesCapacity);
				PX_ASSERT(allNodes[kinematicNodeId].getIsKinematic());
				NodeType nextProxyNodeId=kinematicProxyNextNodes[kinematicNodeId];
				while(nextProxyNodeId!=INVALID_NODE)
				{
					PX_ASSERT(nextProxyNodeId<allNodesCapacity);
					Node& node=allNodes[nextProxyNodeId];
					node.setIsDeleted();
					const IslandType islandId=node.getIslandId();
					PX_ASSERT(islandId<islands.getCapacity());
					graphStartIslands[nextProxyNodeId]=islandId;
					graphNextIslands[nextProxyNodeId]=INVALID_ISLAND;
					kinematicIslandsBitmap.set(islandId);
					nextProxyNodeId=kinematicProxyNextNodes[nextProxyNodeId];
				}
			}//b
		}//w
	}

	//Iterate over all islands with a kinematic proxy and remove them from the node list and from the edges.
	{
		const PxU32 lastSetBit = kinematicIslandsBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicIslandsBitmap.getWords()[w]; b; b &= b-1)
			{
				const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				PX_ASSERT(islandId<islands.getCapacity());

				//Remove all kinematic proxy nodes from all affected islands.
				removeDeletedNodesFromIsland
					(islandId,nodeManager,islands);

				//Make the edges reference the original node.
				const Island& island=islands.get(islandId);
				EdgeType nextEdge=island.mStartEdgeId;
				while(INVALID_EDGE!=nextEdge)
				{
					//Get the edge.
					PX_ASSERT(nextEdge<allEdgesCapacity);
					Edge& edge=allEdges[nextEdge];

					const NodeType node1=edge.getNode1();
					if(INVALID_NODE!=node1)
					{
						PX_ASSERT(node1<allNodesCapacity);
						if(kinematicProxySourceNodes[node1]!=INVALID_NODE)
						{
							PX_ASSERT(allNodes[node1].getIsKinematic());
							PX_ASSERT(allNodes[node1].getIsDeleted());
							edge.setNode1(kinematicProxySourceNodes[node1]);
						}
					}

					const NodeType node2=edge.getNode2();
					if(INVALID_NODE!=node2)
					{
						PX_ASSERT(node2<allNodesCapacity);
						if(kinematicProxySourceNodes[node2]!=INVALID_NODE)
						{
							PX_ASSERT(allNodes[node2].getIsKinematic());
							PX_ASSERT(allNodes[node2].getIsDeleted());
							edge.setNode2(kinematicProxySourceNodes[node2]);
						}
					}

					nextEdge=nextEdgeIds[nextEdge];			
				}//nextEdge
			}//b
		}//w
	}

	//Merge all the islands.
	{
		const PxU32* PX_RESTRICT kinematicNodesBitmapWords=kinematicNodesBitmap.getWords();
		const PxU32 lastSetBit = kinematicNodesBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodesBitmapWords[w]; b; b &= b-1)
			{
				const NodeType kinematicNodeId = (NodeType)(w<<5|Ps::lowestSetBit(b));

				//Get the first proxy and its island.
				const NodeType firstProxyNodeId=kinematicProxyNextNodes[kinematicNodeId];

				if(INVALID_NODE!=firstProxyNodeId)
				{
					//The kinematic is referenced by at least one edge.
					//Safe to get the first proxy node.
					PX_ASSERT(firstProxyNodeId<allNodesCapacity);
					IslandType firstIslandId=INVALID_ISLAND;
					IslandType nextIslandId0=graphStartIslands[firstProxyNodeId];
					PX_ASSERT(nextIslandId0!=INVALID_ISLAND);
					while(nextIslandId0!=INVALID_ISLAND)
					{
						firstIslandId=nextIslandId0;
						nextIslandId0=graphNextIslands[nextIslandId0];
					}
					PX_ASSERT(firstIslandId<islands.getCapacity());

					//First set the node to be in the correct island.
					const NodeType kinematicSourceId=kinematicProxySourceNodes[firstProxyNodeId];
					PX_ASSERT(kinematicSourceId==kinematicNodeId);
					PX_ASSERT(kinematicSourceId<allNodesCapacity);
					Node& kinematicSourceNode=allNodes[kinematicSourceId];
					kinematicSourceNode.setIslandId(firstIslandId);

					//Now add the node to the island.
					PX_ASSERT(kinematicSourceId<allNodesCapacity);
					addNodeToIsland(firstIslandId,kinematicSourceId,allNodes,nextNodeIds,allNodesCapacity,islands);

					//Now merge the other islands containing a kinematic proxy with the first island.
					NodeType nextProxyNodeId=kinematicProxyNextNodes[firstProxyNodeId];
					while(INVALID_NODE!=nextProxyNodeId)
					{
						PX_ASSERT(nextProxyNodeId<allNodesCapacity);

						//Get the island.
						IslandType islandId=INVALID_ISLAND;
						IslandType nextIslandId=graphStartIslands[nextProxyNodeId];
						PX_ASSERT(nextIslandId!=INVALID_ISLAND);
						while(nextIslandId!=INVALID_ISLAND)
						{
							islandId=nextIslandId;
							nextIslandId=graphNextIslands[nextIslandId];
						}
						PX_ASSERT(islandId<islands.getCapacity());

						//Merge the two islands provided they are 
						//(i)  different
						//(ii) islandId hasn't already been merged into another island and released

						if(firstIslandId!=islandId && islands.getBitmap().test(islandId))
						{
							setNodeIslandIdsAndJoinIslands(firstIslandId,islandId,allNodes,allNodesCapacity,allEdges,allEdgesCapacity,nextNodeIds,nextEdgeIds,islands);
							graphNextIslands[islandId]=firstIslandId;
						}

						//Get the next proxy node.			
						nextProxyNodeId=kinematicProxyNextNodes[nextProxyNodeId];
					}
				}//INVALID_NODE!=firstProxyNodeId

				//Nodes with no edges require no treatment.
				//They were already put in their own island when during node duplication.
			}//b
		}//w
	}

	//Release the proxies.
	{
		const PxU32* PX_RESTRICT kinematicNodesBitmapWords=kinematicNodesBitmap.getWords();
		const PxU32 lastSetBit = kinematicNodesBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicNodesBitmapWords[w]; b; b &= b-1)
			{
				const NodeType kinematicNodeId = (NodeType)(w<<5|Ps::lowestSetBit(b));
				NodeType nextProxyNode=kinematicProxyNextNodes[kinematicNodeId];
				while(nextProxyNode!=INVALID_NODE)
				{
					PX_ASSERT(allNodes[nextProxyNode].getIsKinematic());
					PX_ASSERT(allNodes[nextProxyNode].getIsDeleted());
					nodeManager.release(nextProxyNode);
					nextProxyNode=kinematicProxyNextNodes[nextProxyNode];
				}
			}
		}
	}
}


/**
\brief Compute a bitmap of islands from a bitmap of nodes.
\param[in] nodesToProcess is a bitmap of nodes affected by node state changes or by nodes referenced from edge that have experienced a state change.
\param[in] kinematicProxyNextNodeIds is a mapping that iterates over the proxies of kinematic nodes. 
If a kinematic node is affected by a state change we need to mark the islands of all proxy nodes.
kinematicProxyNextNodeIds will be null if this is called before calling duplicateKinematicNodes.  
\param[in] nodeManager is a managed collection of nodes. 
\param[out] processIslandsBitmap is the bitmap of all islands affected by the nodes marked in the bitmap nodesToProcess.
*/
void updateIslandsToProcess(const Cm::BitMap& nodesToProcess, const NodeType* kinematicProxyNextNodeIds, const NodeManager& nodeManager, Cm::BitMap& processIslandsBitmap)
{
	const PxU32 lastSetBit = nodesToProcess.findLast();
	for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
	{
		for(PxU32 b = nodesToProcess.getWords()[w]; b; b &= b-1)
		{
			const IslandType nodeId = (IslandType)(w<<5|Ps::lowestSetBit(b));
			const Node& node = nodeManager.get(nodeId);
			if(!node.getIsKinematic())
			{
				const IslandType islandId = node.getIslandId();
				processIslandsBitmap.set(islandId);
			}
			else
			{
				const IslandType islandId = node.getIslandId();
				if(INVALID_ISLAND != islandId)
				{
					processIslandsBitmap.set(islandId);
				}
				else
				{
					PX_ASSERT(INVALID_NODE != kinematicProxyNextNodeIds[nodeId]);
					NodeType nextProxyNode = kinematicProxyNextNodeIds[nodeId];
					while(nextProxyNode != INVALID_NODE)
					{
						const Node& nextNode = nodeManager.get(nextProxyNode);
						const IslandType nextIslandId = nextNode.getIslandId();
						processIslandsBitmap.set(nextIslandId);
						nextProxyNode = kinematicProxyNextNodeIds[nextProxyNode];
					}
				}
			}
		}
	}
}

#ifdef PX_DEBUG
bool verifyNotReadyForSleepingNodeBitmap(const Cm::BitMap& nodeNotReadyForSleepingBitmap, const NodeManager& nodeManager, const IslandManager& islands)
{
	//Iterate over the bitmap and test that the nodes have the correct flag set.
	{
		const PxU32 lastSetBit = nodeNotReadyForSleepingBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = nodeNotReadyForSleepingBitmap.getWords()[w]; b; b &= b-1)
			{
				const NodeType nodeId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				const Node& node = nodeManager.get(nodeId);
				if(node.getIsReadyForSleeping())
				{
					return false;
				}
				if(node.getIsDeleted())
				{
					return false;
				}
			}
		}
	}

	//Iterate over all nodes and test that the bitmap is correctly set.
	{
		const NodeType* nextNodeIds = nodeManager.getNextNodeIds();
		const Cm::BitMap& islandsBitmap = islands.getBitmap();
		const PxU32 lastSetBit = islandsBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = islandsBitmap.getWords()[w]; b; b &= b-1)
			{
				const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				const Island& island = islands.get(islandId);
				NodeType nextNodeId = island.mStartNodeId;
				while(INVALID_NODE != nextNodeId)
				{
					const Node& node = nodeManager.get(nextNodeId);
					if(!node.getIsDeleted())
					{
						const bool b0 = !node.getIsReadyForSleeping();
						const bool b1 = nodeNotReadyForSleepingBitmap.test(nextNodeId) ? true : false;
						if(b0 != b1)
						{
							return false;
						}
					}
					nextNodeId = nextNodeIds[nextNodeId];
				}
			}
		}
	}

	return true;
}

bool verifyKinaticNodeBitmap(const Cm::BitMap& kinematicBitmap, const NodeManager& nodeManager, const IslandManager& islands)
{
	//Iterate over the bitmap and test that the nodes have the correct flag set.
	{
		const PxU32 lastSetBit = kinematicBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = kinematicBitmap.getWords()[w]; b; b &= b-1)
			{
				const NodeType nodeId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				const Node& node = nodeManager.get(nodeId);
				if(!node.getIsKinematic())
				{
					return false;
				}
				if(node.getIsDeleted())
				{
					return false;
				}
			}
		}
	}

	//Iterate over all nodes and test that the bitmap is correctly set.
	{
		const NodeType* nextNodeIds = nodeManager.getNextNodeIds();
		const Cm::BitMap& islandsBitmap = islands.getBitmap();
		const PxU32 lastSetBit = islandsBitmap.findLast();
		for(PxU32 w = 0; w <= lastSetBit >> 5; ++w)
		{
			for(PxU32 b = islandsBitmap.getWords()[w]; b; b &= b-1)
			{
				const IslandType islandId = (IslandType)(w<<5|Ps::lowestSetBit(b));
				const Island& island = islands.get(islandId);
				NodeType nextNodeId = island.mStartNodeId;
				while(INVALID_NODE != nextNodeId)
				{
					const Node& node = nodeManager.get(nextNodeId);
					if(!node.getIsDeleted())
					{
						const bool b0 = node.getIsKinematic();
						const bool b1 = kinematicBitmap.test(nextNodeId) ? true : false;
						if(b0 != b1)
						{
							return false;
						}
					}
					nextNodeId = nextNodeIds[nextNodeId];
				}
			}
		}
	}

	return true;
}

#endif

#ifndef __SPU__
#define ISLANDGEN_PROFILE 1
#else
#define ISLANDGEN_PROFILE 0
#endif

void physx::updateIslandsMain
(const PxU32 rigidBodyOffset,
 const NodeType* PX_RESTRICT deletedNodes, const PxU32 numDeletedNodes,
 const NodeType* PX_RESTRICT createdNodes, const PxU32 numCreatedNodes,
 const EdgeType* PX_RESTRICT deletedEdges, const PxU32 numDeletedEdges,
 const EdgeType* PX_RESTRICT /*createdEdges*/, const PxU32 /*numCreatedEdges*/,
 const EdgeType* PX_RESTRICT brokenEdges, const PxU32 numBrokenEdges,
 const EdgeType* PX_RESTRICT joinedEdges, const PxU32 numJoinedEdges,
 const Cm::BitMap& kinematicNodesBitmap, const Cm::BitMap& kinematicChangeNodesBitmap, const PxU32 numKinematics,
 const Cm::BitMap& notReadyForSleepingNodesBitmap, const Cm::BitMap& notReadyForSleepingChangeNodesBitmap,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager,
 ProcessSleepingIslandsComputeData& psicData,
 IslandManagerUpdateWorkBuffers& workBuffers,
 Cm::EventProfiler* profiler)
{
#if PX_IS_SPU || !(defined(PX_CHECKED) || defined(PX_PROFILE) || defined(PX_DEBUG))
	PX_UNUSED(profiler);
#endif

	//This function processes created/deleted nodes and broken/joined edges to create a list of connected islands with the caveat that 
	//kinematics nodes don't act as bridges between islands ie if B is kinematic and we have edges A-B and B-C then we have two separate 
	//islands (A,B) and (B,C).
	//The final list of islands is parsed to determine which islands are asleep and which are awake so that sleep state changes may be reported 
	//and nodes/edges in awake islands can be sent to the solver.  This is done in the function processSleepingIslands.

	//The function processSleepingIslands achieves two goals
	//(i)  Determine lists of nodes with modified sleep state.  These lists are stored so that they may be externally queried later.
	//(ii) Parse all awake islands to generate lists of nodes and edges for the solver. 
	//Every operation in updateIslandsMain can be thought of as preparing an updated list of islands so that processSleepingIslands can 
	//be successfully called.  This involves processing all state changes to nodes and edges that occurred since the last island update.

	//When the list of islands is parsed in processSleepingIslands we don't need to iterate over all islands in the list.  
	//The key point here is we are are only interested in reporting changes to node sleep states and in reporting the nodes and edges of awake islands.
	//More specifically we are interested in islands that 
	//(i)   contain a node that changed kinematic state 
	//			(PxsIslandManager::setKinematic)
	//(ii)  contain a node that changed not-ready-for-sleeping state 
	//			(PxsIslandManager::notifyReadyForSleeping, PxsIslandManager::notifyNotReadyForSleeping, PxsIslandManager::setAsleep, PxsIslandManager::setAwake)
	//(iii) contain a node in the created list
	//			(PxsIslandManager::addBody)
	//(iv)  contain an edge in the joined edge list
	//			(we'll need to work this out from the list of joined edges)
	//(v)   contain an edge in the broken edge list
	//			(we'll need to work this out from the lists of broken/deleted edges)
	//(vi)  contain a node that is not-ready-for-sleeping
	//			(PxsIslandManager::notifyNotReadyForSleeping, PxsIslandManager::setAwake)

	//The algorithm proceeds by
	//(a)   Computing a bit map of nodes that are of interest to us.  The bitmap nodeStateChangeBitmap describes the union of all nodes
	//in the created list and all nodes that changed kinematic state and all nodes that changed not-ready-for-sleeping state. The bitmap
	//nodeNotReadyForSleepingBitmap describes all nodes that cannot fall asleep.  We can directly compute the union of these two 
	//bitmaps.  All nodes affected by broken, deleted and joined edges can be added to this union.  All nodes deleted are removed from the bitmap.
	//We store the result in nodeStateChangeBitmap.
	//An optimization is to add to nodeStateChangeBitmap as we naturally process the modified edges (stages (b) and (c)) and deleted nodes (stage (e)).
	//(b)   Remove all deleted nodes from their islands and remove all broken edges from the islands.  Empty islands are released for re-use.
	//      Islands affected by a broken edge are recorded in brokenEdgeIslandsBitmap.
	//(c)   Update all islands affected by edges in the joined list.  brokenEdgeIslandsBitmap is updated to account for any islands 
	//      that are discarded when islands are merged as a result of a joined edge.  Note that joined edges might reference a node in the 
	//      created node list.
	//(d)   Every created node that was not managed by processJoinedEdges is placed in its own island.  These nodes must be referenced by no joined edges.
	//(e)   Nodes/edges that are in the deleted node/edge lists are recycled.
	//(f)   To enforce the rule that kinematic nodes don't act as the bridge between two islands we replace each kinematic node with 
	//      a set of proxy nodes: one proxy node for each edge referencing a kinematic. We don't need to do this for all kinematic nodes 
	//      because we are only interested in kinematics that are in the set of islands that will be parsed in processSleepingIslands.
	//      It is straightforward to compute the list of islands that will be parsed in processSleepingIslands and then only consider 
	//      those islands that contain a kinematic.  This subset of islands-containing-kinematics have effectively experienced a broken edge 
	//      event and need to be added to brokenEdgeIslandsBitmap. 
	//(g)  All islands in brokenEdgeIslandsBitmap are recomputed from their edge lists.  This typically has the effect of increasing the 
	//      number of islands.  The islands generated by this process are stored in processIslandsBitmap.
	//(h) The list of islands to parse in processSleepingIslands is recomputed and processSleepingIslands is called.

	//Note that stage (f) introduces proxy nodes that need to be replaced with the source kinematic nodes at some point before calling 
	//updateIslandsMain again.  This is achieved with the function mergeKinematicProxiesBackToSource and is called from freeBuffers.

	PX_ASSERT(verifyNotReadyForSleepingNodeBitmap(notReadyForSleepingNodesBitmap, nodeManager, islands));
	PX_ASSERT(verifyKinaticNodeBitmap(kinematicNodesBitmap, nodeManager, islands));

	Cm::BitMap& brokenEdgeIslandsBitmap = *workBuffers.mBitmap[IslandManagerUpdateWorkBuffers::eBROKEN_EDGE_ISLANDS];
	brokenEdgeIslandsBitmap.clearFast();

	Cm::BitMap& processIslandsBitmap = *workBuffers.mBitmap[IslandManagerUpdateWorkBuffers::ePROCESS_ISLANDS];
	processIslandsBitmap.clearFast();

	Cm::BitMap& nodeStateChangeBitmap = *workBuffers.mBitmap[IslandManagerUpdateWorkBuffers::eCHANGED_NODES];
	nodeStateChangeBitmap.clearFast();

	//*************************************************************************
	//Stage (a) - Computing a bit map of nodes that are of interest to us.
	//For the time being we just compute the union of nodes that changed state and 
	//nodes that are flagged not-ready-for-sleeping.  Nodes in broken/deleted edges are
	//added in stage (b) (removeBrokenEdgesFromIslands), nodes in joined edges are
	//added in stage (c) (processJoinedEdges), nodes that have been deleted are 
	//removed in stage (e) (releaseDeletedNodes).  Note that we always account for 
	//deleted nodes last because an edge could be modified and have a node deleted.
	//*************************************************************************
	{
		//nodeStateChangeBitmap = notReadyForSleepingChangeNodesBitmap | kinematicChangeNodesBitmap | notReadyForSleepingNodesBitmap
		combineOr(notReadyForSleepingChangeNodesBitmap, nodeStateChangeBitmap);
		combineOr(kinematicChangeNodesBitmap, nodeStateChangeBitmap);
		combineOr(notReadyForSleepingNodesBitmap, nodeStateChangeBitmap);
		for(PxU32 i = 0; i < numCreatedNodes; i++)
		{
			const NodeType nodeId = createdNodes[i];
			nodeStateChangeBitmap.set(nodeId);
		}
	}

	//*******************************************************************************************************
	//Stage (b) - Remove all deleted nodes from their islands and remove all broken edges from the islands.
	//********************************************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetemptyIslands());
#endif

		//Use processIslandsBitmap locally to record empty islands.
		Cm::BitMap& emptyIslandsBitmap = processIslandsBitmap;

		//When removing deleted nodes from islands some islands end up empty.
		//Don't immediately release these islands because we also want to 
		//remove edges from islands too and it makes it harder to manage
		//the book-keeping if some islands have already been released.
		//Store a list of empty islands and release after removing 
		//deleted edges from islands.

		//Remove deleted nodes from islands.
		{
			Cm::BitMap& scratchBitmap = brokenEdgeIslandsBitmap;
			PX_ASSERT(0 == scratchBitmap.findLast());

			removeDeletedNodesFromIslands(
				deletedNodes, numDeletedNodes,
				nodeManager, islands,
				scratchBitmap, emptyIslandsBitmap);

			scratchBitmap.clearFast();
		}

		//Remove broken/deleted edges from islands.
		{
			PX_ASSERT(0 == brokenEdgeIslandsBitmap.findLast());
			removeBrokenEdgesFromIslands(
				brokenEdges, numBrokenEdges,
				deletedEdges, numDeletedEdges,
				NULL,
				nodeManager, edgeManager, islands,
				brokenEdgeIslandsBitmap,
				&nodeStateChangeBitmap);
		}

		//Now release all empty islands.
		{
			releaseEmptyIslands(
				emptyIslandsBitmap,
				islands,
				brokenEdgeIslandsBitmap);
		}

		//Make sure to clear out the empty islands.
		emptyIslandsBitmap.clearFast();
		PX_ASSERT(0 == processIslandsBitmap.findLast());

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetemptyIslands());
#endif
	}

	//*************************************************************************
	//Stage (c) - Update all islands affected by edges in the joined list.
	//brokenEdgeIslandsBitmap is updated to account for any islands 
	//that are discarded when islands are merged as a result of a joined edge.
	//*************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetjoinedEdges());
#endif

		Cm::BitMap& scratchBitmap = processIslandsBitmap;
		PX_ASSERT(0 == scratchBitmap.findLast());

		NodeType* graphNextNodes=workBuffers.mGraphNextNodes;
		IslandType* graphStartIslands=workBuffers.mGraphStartIslands;
		IslandType* graphNextIslands=workBuffers.mGraphNextIslands;

		processJoinedEdges(
			joinedEdges, numJoinedEdges,
			nodeManager, edgeManager, islands,
			brokenEdgeIslandsBitmap,
			scratchBitmap,
			graphNextNodes, graphStartIslands, graphNextIslands,
			nodeStateChangeBitmap);

		//Make sure to clear out the work buffer.
		scratchBitmap.clearFast();
		PX_ASSERT(0 == processIslandsBitmap.findLast());

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetjoinedEdges());
#endif
	}

	//*************************************************************************
	//Stage (d) - Every created node that was not managed by processJoinedEdges
	//is placed in its own island.	
	//*************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetcreatedNodes());
#endif

		processCreatedNodes(
			createdNodes, numCreatedNodes,
			nodeManager, islands);

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetcreatedNodes());
#endif
	}

	//*****************************************************************************
	//Stage (e) - Nodes/edges that are in the deleted node/edge lists are recycled.
	//*****************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetdeletedNodesEdges());
#endif

		releaseDeletedNodes(
			deletedNodes, numDeletedNodes, 
			nodeManager, nodeStateChangeBitmap);

		releaseDeletedEdges(
			deletedEdges,numDeletedEdges,
			edgeManager);

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetdeletedNodesEdges());
#endif
	}
	PX_ASSERT(0 == processIslandsBitmap.findLast());

	//**************************************************************************************************************
	//Stage (f) - To enforce the rule that kinematic nodes don't act as the bridge between two islands we replace 
	//each kinematic node with  a set of proxy nodes with one proxy node for each edge referencing a kinematic.
	//The subset of islands-containing-kinematics have effectively experienced a broken edge 
	//event and need to be added to brokenEdgeIslandsBitmap. 
	//**************************************************************************************************************
	if(numKinematics > 0)
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetduplicateKinematicNodes());
#endif

		//Get a list of all islands that need processed.
		//(We're only interested in islands that have a kinematic and need processed).
		updateIslandsToProcess(nodeStateChangeBitmap, NULL, nodeManager, processIslandsBitmap);

		//A mapping between source kinematic nodes and their proxies. 
		NodeType* kinematicProxySourceNodeIds=workBuffers.mKinematicProxySourceNodeIds;
		NodeType* kinematicProxyNextNodeIds=workBuffers.mKinematicProxyNextNodeIds;
		NodeType* kinematicProxyLastNodeIds=workBuffers.mKinematicProxyLastNodeIds;

		PxU8 scratchBitmapBuffer[sizeof(Cm::BitMap)];
		Cm::BitMap* scratchBitmap = (Cm::BitMap*)scratchBitmapBuffer;
		scratchBitmap->setWords((PxU32*)workBuffers.mGraphNextNodes, nodeManager.getCapacity() >> 5);
		scratchBitmap->clearFast();

		//Replace all kinematics with proxies.
		//Update brokenEdgeIslandsBitmap with all affected islands.
		duplicateKinematicNodes(
			kinematicNodesBitmap, processIslandsBitmap,
			nodeManager,edgeManager,islands,
			kinematicProxySourceNodeIds,kinematicProxyNextNodeIds,kinematicProxyLastNodeIds,
			brokenEdgeIslandsBitmap, *scratchBitmap);

		//We don't need this any more (or rather it will be out of date soon).
		processIslandsBitmap.clearFast();

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetduplicateKinematicNodes());
#endif
	}
	PX_ASSERT(0 == processIslandsBitmap.findLast());

	//*********************************************************************************
	//(g)  All islands in brokenEdgeIslandsBitmap are recomputed from their edge lists.
	//All islands generated by this process is recorded in processIslandsBitmap.
	//*********************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetbrokenEdgeIslands());
#endif

		//Acceleration data.
		NodeType* graphNextNodes=workBuffers.mGraphNextNodes;
		IslandType* graphStartIslands=workBuffers.mGraphStartIslands;
		IslandType* graphNextIslands=workBuffers.mGraphNextIslands;

		processBrokenEdgeIslands(
			brokenEdgeIslandsBitmap,
			nodeManager,edgeManager,islands,
			graphNextNodes,graphStartIslands,graphNextIslands,
			processIslandsBitmap);

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetbrokenEdgeIslands());
#endif
	}

	//*********************************************************************************
	//Stage (h) - The list of islands to parse in processSleepingIslands is recomputed 
	//and processSleepingIslands is called.	
	//*********************************************************************************
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetprocessSleepingIslands());
#endif

		//Compute all islands to process in processSleepingIslands
		updateIslandsToProcess(nodeStateChangeBitmap, workBuffers.mKinematicProxyNextNodeIds, nodeManager, processIslandsBitmap);
		
#ifndef __SPU__
		processSleepingIslands<!tSecondPass>(
			processIslandsBitmap, rigidBodyOffset,
			nodeManager,edgeManager,islands,articulationRootManager,
			workBuffers.mKinematicProxySourceNodeIds, 
			psicData);
#else
		processSleepingIslands(
			islands.getBitmap(), rigidBodyOffset,
			nodeManager,edgeManager,islands,articulationRootManager,
			workBuffers.mKinematicProxySourceNodeIds, 
			psicData);
#endif

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetprocessSleepingIslands());
#endif
	}
}

void physx::updateIslandsSecondPassMain
(const PxU32 rigidBodyOffset, Cm::BitMap& processIslandsBitmap,
 const EdgeType* PX_RESTRICT brokenEdges, const PxU32 numBrokenEdges,
 NodeManager& nodeManager, EdgeManager& edgeManager, IslandManager& islands, ArticulationRootManager& articulationRootManager,
 ProcessSleepingIslandsComputeData& psicData,
 IslandManagerUpdateWorkBuffers& workBuffers,
 Cm::EventProfiler* profiler)
{
#if PX_IS_SPU || !(defined(PX_CHECKED) || defined(PX_PROFILE) || defined(PX_DEBUG))
	PX_UNUSED(profiler);
#endif

	PX_ASSERT(&processIslandsBitmap == workBuffers.mBitmap[IslandManagerUpdateWorkBuffers::ePROCESS_ISLANDS]);

	//Bitmaps of islands affected by broken edges.
	Cm::BitMap& brokenEdgeIslandsBitmap=*workBuffers.mBitmap[IslandManagerUpdateWorkBuffers::eBROKEN_EDGE_ISLANDS];
	brokenEdgeIslandsBitmap.clearFast();

	//Remove broken edges from islands.
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetemptyIslands());
#endif

		//Remove broken/deleted edges from islands.
		removeBrokenEdgesFromIslands(
			brokenEdges,numBrokenEdges,
			NULL,0,
			workBuffers.mKinematicProxySourceNodeIds,
			nodeManager,edgeManager,islands,
			brokenEdgeIslandsBitmap, NULL);

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetemptyIslands());
#endif
	}

	//Any islands with a broken edge need to be rebuilt into their sub-islands.
	//processIslandsBitmap needs updated to reflect the changes to active island ids.
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetbrokenEdgeIslands());
#endif

		NodeType* graphNextNodes=workBuffers.mGraphNextNodes;
		IslandType* graphStartIslands=workBuffers.mGraphStartIslands;
		IslandType* graphNextIslands=workBuffers.mGraphNextIslands;

		processBrokenEdgeIslands(
			brokenEdgeIslandsBitmap,
			nodeManager,edgeManager,islands,
			graphNextNodes,graphStartIslands,graphNextIslands,
			processIslandsBitmap);

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetbrokenEdgeIslands());
#endif
	}

	//Process all the sleeping and awake islands to compute the solver islands data.
	{
#if ISLANDGEN_PROFILE
		CM_PROFILE_START(profiler, Cm::ProfileEventId::IslandGen::GetprocessSleepingIslands());
#endif

#ifndef __SPU__
		processSleepingIslands<tSecondPass>(
			processIslandsBitmap, rigidBodyOffset,
			nodeManager,edgeManager,islands,articulationRootManager,
			workBuffers.mKinematicProxySourceNodeIds,
			psicData);
#else
		processSleepingIslandsSecondPass(
			processIslandsBitmap, rigidBodyOffset,
			nodeManager,edgeManager,islands,articulationRootManager,
			workBuffers.mKinematicProxySourceNodeIds,
			psicData);
#endif

#if ISLANDGEN_PROFILE
		CM_PROFILE_STOP(profiler, Cm::ProfileEventId::IslandGen::GetprocessSleepingIslands());
#endif
	}
}
