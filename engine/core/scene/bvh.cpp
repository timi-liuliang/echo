#include "bvh.h"
#include "engine/core/geom/Ray.h"

#define NullNode -1

/// This is used to fatten AABBs in the dynamic tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
/// This is in meters.
#define AABBExtension 0.1f

/// This is used to fatten AABBs in the dynamic tree. This is used to predict
/// the future position based on the current displacement.
/// This is a dimensionless multiplier.
#define AABBMultiplier		2.0f

namespace Echo
{
	Bvh::Bvh()
	{
		m_root = -1;

		m_nodeCapacity = 16;
		m_nodeCount = 0;
		m_nodes = (BvhNode*)EchoAlloc(BvhNode, m_nodeCapacity);
		memset(m_nodes, 0, m_nodeCapacity * sizeof(BvhNode));

		// Build a linked list for the free list.
		for (i32 i = 0; i < m_nodeCapacity - 1; ++i)
		{
			m_nodes[i].next = i + 1;
			m_nodes[i].height = -1;
		}
		m_nodes[m_nodeCapacity - 1].next = -1;
		m_nodes[m_nodeCapacity - 1].height = -1;
		m_freeList = 0;

		m_path = 0;

		m_insertionCount = 0;
	}

	Bvh::~Bvh()
	{
		// This frees the entire tree in one shot.
		EchoSafeFree(m_nodes);
	}

	i32 Bvh::allocateNode()
	{
		// Expand the node pool as needed.
		if (m_freeList == -1)
		{
			EchoAssert(m_nodeCount == m_nodeCapacity);

			// The free list is empty. Rebuild a bigger pool.
			BvhNode* oldNodes = m_nodes;
			m_nodeCapacity *= 2;
			m_nodes = (BvhNode*)EchoAlloc(BvhNode, m_nodeCapacity);
			memcpy(m_nodes, oldNodes, m_nodeCount * sizeof(BvhNode));
			EchoSafeFree(oldNodes);

			// Build a linked list for the free list. The parent
			// pointer becomes the "next" pointer.
			for (i32 i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
			{
				m_nodes[i].next = i + 1;
				m_nodes[i].height = -1;
			}
			m_nodes[m_nodeCapacity - 1].next = -1;
			m_nodes[m_nodeCapacity - 1].height = -1;
			m_freeList = m_nodeCount;
		}

		// Peel a node off the free list.
		i32 nodeId = m_freeList;
		m_freeList = m_nodes[nodeId].next;
		m_nodes[nodeId].parent = NullNode;
		m_nodes[nodeId].child1 = NullNode;
		m_nodes[nodeId].child2 = NullNode;
		m_nodes[nodeId].height = 0;
		m_nodes[nodeId].userData = NULL;
		++m_nodeCount;
		return nodeId;
	}

	// Return a node to the pool.
	void Bvh::freeNode(i32 nodeId)
	{
		EchoAssert(0 <= nodeId && nodeId < m_nodeCapacity);
		EchoAssert(0 < m_nodeCount);
		m_nodes[nodeId].next = m_freeList;
		m_nodes[nodeId].height = -1;
		m_freeList = nodeId;
		--m_nodeCount;
	}

	// Create a proxy in the tree as a leaf node. We return the index
	// of the node instead of a pointer so that we can grow
	// the node pool.
	i32 Bvh::createProxy(const AABB& aabb, void* userData)
	{
		i32 proxyId = allocateNode();

		// Fatten the aabb.
		Vector3 r(AABBExtension, AABBExtension, AABBExtension);
		m_nodes[proxyId].aabb.vMin = aabb.vMin - r;
		m_nodes[proxyId].aabb.vMax = aabb.vMax + r;
		m_nodes[proxyId].userData = userData;
		m_nodes[proxyId].height = 0;

		insertLeaf(proxyId);

		return proxyId;
	}

	void Bvh::destroyProxy(i32 proxyId)
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);
		EchoAssert(m_nodes[proxyId].IsLeaf());

		removeLeaf(proxyId);
		freeNode(proxyId);
	}

	bool Bvh::moveProxy(i32 proxyId, const AABB& aabb, const Vector3& displacement)
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);

		EchoAssert(m_nodes[proxyId].IsLeaf());

		if (m_nodes[proxyId].aabb.isContain(aabb))
		{
			return false;
		}

		removeLeaf(proxyId);

		// Extend AABB.
		AABB b = aabb;
		Vector3 r(AABBExtension, AABBExtension, AABBExtension);
		b.vMin = b.vMin - r;
		b.vMax = b.vMax + r;

		// Predict AABB displacement.
		Vector3 d = AABBMultiplier * displacement;

		if (d.x < 0.0f)
		{
			b.vMin.x += d.x;
		}
		else
		{
			b.vMax.x += d.x;
		}

		if (d.y < 0.0f)
		{
			b.vMin.y += d.y;
		}
		else
		{
			b.vMax.y += d.y;
		}

		if (d.z < 0.0f)
		{
			b.vMin.z += d.z;
		}
		else
		{
			b.vMax.z += d.z;
		}

		m_nodes[proxyId].aabb = b;

		insertLeaf(proxyId);
		return true;
	}

	void Bvh::insertLeaf(i32 leaf)
	{
		++m_insertionCount;

		if (m_root == NullNode)
		{
			m_root = leaf;
			m_nodes[m_root].parent = NullNode;
			return;
		}

		// Find the best sibling for this node
		AABB leafAABB = m_nodes[leaf].aabb;
		i32 index = m_root;
		while (m_nodes[index].IsLeaf() == false)
		{
			i32 child1 = m_nodes[index].child1;
			i32 child2 = m_nodes[index].child2;

			float area = m_nodes[index].aabb.getPerimeter();

			AABB combinedAABB;
			combinedAABB.unionBox(m_nodes[index].aabb, leafAABB);
			float combinedArea = combinedAABB.getPerimeter();

			// Cost of creating a new parent for this node and the new leaf
			float cost = 2.0f * combinedArea;

			// Minimum cost of pushing the leaf further down the tree
			float inheritanceCost = 2.0f * (combinedArea - area);

			// Cost of descending into child1
			float cost1;
			if (m_nodes[child1].IsLeaf())
			{
				AABB aabb;
				aabb.unionBox(leafAABB, m_nodes[child1].aabb);
				cost1 = aabb.getPerimeter() + inheritanceCost;
			}
			else
			{
				AABB aabb;
				aabb.unionBox(leafAABB, m_nodes[child1].aabb);
				float oldArea = m_nodes[child1].aabb.getPerimeter();
				float newArea = aabb.getPerimeter();
				cost1 = (newArea - oldArea) + inheritanceCost;
			}

			// Cost of descending into child2
			float cost2;
			if (m_nodes[child2].IsLeaf())
			{
				AABB aabb;
				aabb.unionBox(leafAABB, m_nodes[child2].aabb);
				cost2 = aabb.getPerimeter() + inheritanceCost;
			}
			else
			{
				AABB aabb;
				aabb.unionBox(leafAABB, m_nodes[child2].aabb);
				float oldArea = m_nodes[child2].aabb.getPerimeter();
				float newArea = aabb.getPerimeter();
				cost2 = newArea - oldArea + inheritanceCost;
			}

			// Descend according to the minimum cost.
			if (cost < cost1 && cost < cost2)
			{
				break;
			}

			// Descend
			if (cost1 < cost2)
			{
				index = child1;
			}
			else
			{
				index = child2;
			}
		}

		i32 sibling = index;

		// Create a new parent.
		i32 oldParent = m_nodes[sibling].parent;
		i32 newParent = allocateNode();
		m_nodes[newParent].parent = oldParent;
		m_nodes[newParent].userData = NULL;
		m_nodes[newParent].aabb.unionBox(leafAABB, m_nodes[sibling].aabb);
		m_nodes[newParent].height = m_nodes[sibling].height + 1;

		if (oldParent != NullNode)
		{
			// The sibling was not the root.
			if (m_nodes[oldParent].child1 == sibling)
			{
				m_nodes[oldParent].child1 = newParent;
			}
			else
			{
				m_nodes[oldParent].child2 = newParent;
			}

			m_nodes[newParent].child1 = sibling;
			m_nodes[newParent].child2 = leaf;
			m_nodes[sibling].parent = newParent;
			m_nodes[leaf].parent = newParent;
		}
		else
		{
			// The sibling was the root.
			m_nodes[newParent].child1 = sibling;
			m_nodes[newParent].child2 = leaf;
			m_nodes[sibling].parent = newParent;
			m_nodes[leaf].parent = newParent;
			m_root = newParent;
		}

		// Walk back up the tree fixing heights and AABBs
		index = m_nodes[leaf].parent;
		while (index != NullNode)
		{
			index = balance(index);

			i32 child1 = m_nodes[index].child1;
			i32 child2 = m_nodes[index].child2;

			EchoAssert(child1 != NullNode);
			EchoAssert(child2 != NullNode);

			m_nodes[index].height = 1 + Math::Max<i32>(m_nodes[child1].height, m_nodes[child2].height);
			m_nodes[index].aabb.unionBox(m_nodes[child1].aabb, m_nodes[child2].aabb);

			index = m_nodes[index].parent;
		}

		//Validate();
	}

	void Bvh::removeLeaf(i32 leaf)
	{
		if (leaf == m_root)
		{
			m_root = NullNode;
			return;
		}

		i32 parent = m_nodes[leaf].parent;
		i32 grandParent = m_nodes[parent].parent;
		i32 sibling;
		if (m_nodes[parent].child1 == leaf)
		{
			sibling = m_nodes[parent].child2;
		}
		else
		{
			sibling = m_nodes[parent].child1;
		}

		if (grandParent != NullNode)
		{
			// Destroy parent and connect sibling to grandParent.
			if (m_nodes[grandParent].child1 == parent)
			{
				m_nodes[grandParent].child1 = sibling;
			}
			else
			{
				m_nodes[grandParent].child2 = sibling;
			}
			m_nodes[sibling].parent = grandParent;
			freeNode(parent);

			// Adjust ancestor bounds.
			i32 index = grandParent;
			while (index != NullNode)
			{
				index = balance(index);

				i32 child1 = m_nodes[index].child1;
				i32 child2 = m_nodes[index].child2;

				m_nodes[index].aabb.unionBox(m_nodes[child1].aabb, m_nodes[child2].aabb);
				m_nodes[index].height = 1 + Math::Max<i32>(m_nodes[child1].height, m_nodes[child2].height);

				index = m_nodes[index].parent;
			}
		}
		else
		{
			m_root = sibling;
			m_nodes[sibling].parent = NullNode;
			freeNode(parent);
		}

		//Validate();
	}

	// Perform a left or right rotation if node A is imbalanced.
	// Returns the new root index.
	i32 Bvh::balance(i32 iA)
	{
		EchoAssert(iA != NullNode);

		BvhNode* A = m_nodes + iA;
		if (A->IsLeaf() || A->height < 2)
		{
			return iA;
		}

		i32 iB = A->child1;
		i32 iC = A->child2;
		EchoAssert(0 <= iB && iB < m_nodeCapacity);
		EchoAssert(0 <= iC && iC < m_nodeCapacity);

		BvhNode* B = m_nodes + iB;
		BvhNode* C = m_nodes + iC;

		i32 balance = C->height - B->height;

		// Rotate C up
		if (balance > 1)
		{
			i32 iF = C->child1;
			i32 iG = C->child2;
			BvhNode* F = m_nodes + iF;
			BvhNode* G = m_nodes + iG;
			EchoAssert(0 <= iF && iF < m_nodeCapacity);
			EchoAssert(0 <= iG && iG < m_nodeCapacity);

			// Swap A and C
			C->child1 = iA;
			C->parent = A->parent;
			A->parent = iC;

			// A's old parent should point to C
			if (C->parent != NullNode)
			{
				if (m_nodes[C->parent].child1 == iA)
				{
					m_nodes[C->parent].child1 = iC;
				}
				else
				{
					EchoAssert(m_nodes[C->parent].child2 == iA);
					m_nodes[C->parent].child2 = iC;
				}
			}
			else
			{
				m_root = iC;
			}

			// Rotate
			if (F->height > G->height)
			{
				C->child2 = iF;
				A->child2 = iG;
				G->parent = iA;
				A->aabb.unionBox(B->aabb, G->aabb);
				C->aabb.unionBox(A->aabb, F->aabb);

				A->height = 1 + Math::Max<i32>(B->height, G->height);
				C->height = 1 + Math::Max<i32>(A->height, F->height);
			}
			else
			{
				C->child2 = iG;
				A->child2 = iF;
				F->parent = iA;
				A->aabb.unionBox(B->aabb, F->aabb);
				C->aabb.unionBox(A->aabb, G->aabb);

				A->height = 1 + Math::Max<i32>(B->height, F->height);
				C->height = 1 + Math::Max<i32>(A->height, G->height);
			}

			return iC;
		}

		// Rotate B up
		if (balance < -1)
		{
			i32 iD = B->child1;
			i32 iE = B->child2;
			BvhNode* D = m_nodes + iD;
			BvhNode* E = m_nodes + iE;
			EchoAssert(0 <= iD && iD < m_nodeCapacity);
			EchoAssert(0 <= iE && iE < m_nodeCapacity);

			// Swap A and B
			B->child1 = iA;
			B->parent = A->parent;
			A->parent = iB;

			// A's old parent should point to B
			if (B->parent != NullNode)
			{
				if (m_nodes[B->parent].child1 == iA)
				{
					m_nodes[B->parent].child1 = iB;
				}
				else
				{
					EchoAssert(m_nodes[B->parent].child2 == iA);
					m_nodes[B->parent].child2 = iB;
				}
			}
			else
			{
				m_root = iB;
			}

			// Rotate
			if (D->height > E->height)
			{
				B->child2 = iD;
				A->child1 = iE;
				E->parent = iA;
				A->aabb.unionBox(C->aabb, E->aabb);
				B->aabb.unionBox(A->aabb, D->aabb);

				A->height = 1 + Math::Max<i32>(C->height, E->height);
				B->height = 1 + Math::Max<i32>(A->height, D->height);
			}
			else
			{
				B->child2 = iE;
				A->child1 = iD;
				D->parent = iA;
				A->aabb.unionBox(C->aabb, D->aabb);
				B->aabb.unionBox(A->aabb, E->aabb);

				A->height = 1 + Math::Max<i32>(C->height, D->height);
				B->height = 1 + Math::Max<i32>(A->height, E->height);
			}

			return iB;
		}

		return iA;
	}

	i32 Bvh::getHeight() const
	{
		if (m_root == NullNode)
		{
			return 0;
		}

		return m_nodes[m_root].height;
	}

	float Bvh::getAreaRatio() const
	{
		if (m_root == NullNode)
		{
			return 0.0f;
		}

		const BvhNode* root = m_nodes + m_root;
		float rootArea = root->aabb.getPerimeter();

		float totalArea = 0.0f;
		for (i32 i = 0; i < m_nodeCapacity; ++i)
		{
			const BvhNode* node = m_nodes + i;
			if (node->height < 0)
			{
				// Free node in pool
				continue;
			}

			totalArea += node->aabb.getPerimeter();
		}

		return totalArea / rootArea;
	}

	i32 Bvh::computeHeight(i32 nodeId) const
	{
		EchoAssert(0 <= nodeId && nodeId < m_nodeCapacity);
		BvhNode* node = m_nodes + nodeId;

		if (node->IsLeaf())
		{
			return 0;
		}

		i32 height1 = computeHeight(node->child1);
		i32 height2 = computeHeight(node->child2);
		return 1 + Math::Max<i32>(height1, height2);
	}

	i32 Bvh::computeHeight() const
	{
		i32 height = computeHeight(m_root);
		return height;
	}

	void Bvh::validateStructure(i32 index) const
	{
		if (index == NullNode)
		{
			return;
		}

		if (index == m_root)
		{
			EchoAssert(m_nodes[index].parent == NullNode);
		}

		const BvhNode* node = m_nodes + index;

		i32 child1 = node->child1;
		i32 child2 = node->child2;

		if (node->IsLeaf())
		{
			EchoAssert(child1 == NullNode);
			EchoAssert(child2 == NullNode);
			EchoAssert(node->height == 0);
			return;
		}

		EchoAssert(0 <= child1 && child1 < m_nodeCapacity);
		EchoAssert(0 <= child2 && child2 < m_nodeCapacity);

		EchoAssert(m_nodes[child1].parent == index);
		EchoAssert(m_nodes[child2].parent == index);

		validateStructure(child1);
		validateStructure(child2);
	}

	void Bvh::validateMetrics(i32 index) const
	{
		if (index == NullNode)
		{
			return;
		}

		const BvhNode* node = m_nodes + index;

		i32 child1 = node->child1;
		i32 child2 = node->child2;

		if (node->IsLeaf())
		{
			EchoAssert(child1 == NullNode);
			EchoAssert(child2 == NullNode);
			EchoAssert(node->height == 0);
			return;
		}

		EchoAssert(0 <= child1 && child1 < m_nodeCapacity);
		EchoAssert(0 <= child2 && child2 < m_nodeCapacity);

		i32 height1 = m_nodes[child1].height;
		i32 height2 = m_nodes[child2].height;
		i32 height;
		height = 1 + Math::Max<i32>(height1, height2);
		EchoAssert(node->height == height);

		AABB aabb;
		aabb.unionBox(m_nodes[child1].aabb, m_nodes[child2].aabb);

		EchoAssert(aabb.vMin == node->aabb.vMin);
		EchoAssert(aabb.vMax == node->aabb.vMax);

		validateMetrics(child1);
		validateMetrics(child2);
	}

	void Bvh::validate() const
	{
		validateStructure(m_root);
		validateMetrics(m_root);

		i32 freeCount = 0;
		i32 freeIndex = m_freeList;
		while (freeIndex != NullNode)
		{
			EchoAssert(0 <= freeIndex && freeIndex < m_nodeCapacity);
			freeIndex = m_nodes[freeIndex].next;
			++freeCount;
		}

		EchoAssert(getHeight() == computeHeight());

		EchoAssert(m_nodeCount + freeCount == m_nodeCapacity);
	}

	i32 Bvh::getMaxBalance() const
	{
		i32 maxBalance = 0;
		for (i32 i = 0; i < m_nodeCapacity; ++i)
		{
			const BvhNode* node = m_nodes + i;
			if (node->height <= 1)
			{
				continue;
			}

			EchoAssert(node->IsLeaf() == false);

			i32 child1 = node->child1;
			i32 child2 = node->child2;
			i32 balance = Math::Abs<i32>(m_nodes[child2].height - m_nodes[child1].height);
			maxBalance = Math::Max<i32>(maxBalance, balance);
		}

		return maxBalance;
	}

	void Bvh::rebuildBottomUp()
	{
		i32* nodes = (i32*)EchoAlloc(i32, m_nodeCount);
		i32 count = 0;

		// Build array of leaves. Free the rest.
		for (i32 i = 0; i < m_nodeCapacity; ++i)
		{
			if (m_nodes[i].height < 0)
			{
				// free node in pool
				continue;
			}

			if (m_nodes[i].IsLeaf())
			{
				m_nodes[i].parent = NullNode;
				nodes[count] = i;
				++count;
			}
			else
			{
				freeNode(i);
			}
		}

		while (count > 1)
		{
			float minCost = FLT_MAX;
			i32 iMin = -1, jMin = -1;
			for (i32 i = 0; i < count; ++i)
			{
				AABB aabbi = m_nodes[nodes[i]].aabb;

				for (i32 j = i + 1; j < count; ++j)
				{
					AABB aabbj = m_nodes[nodes[j]].aabb;
					AABB b;
					b.unionBox(aabbi, aabbj);
					float cost = b.getPerimeter();
					if (cost < minCost)
					{
						iMin = i;
						jMin = j;
						minCost = cost;
					}
				}
			}

			i32 index1 = nodes[iMin];
			i32 index2 = nodes[jMin];
			BvhNode* child1 = m_nodes + index1;
			BvhNode* child2 = m_nodes + index2;

			i32 parentIndex = allocateNode();
			BvhNode* parent = m_nodes + parentIndex;
			parent->child1 = index1;
			parent->child2 = index2;
			parent->height = 1 + Math::Max<i32>(child1->height, child2->height);
			parent->aabb.unionBox(child1->aabb, child2->aabb);
			parent->parent = NullNode;

			child1->parent = parentIndex;
			child2->parent = parentIndex;

			nodes[jMin] = nodes[count - 1];
			nodes[iMin] = parentIndex;
			--count;
		}

		m_root = nodes[0];
		EchoSafeFree(nodes);

		validate();
	}

	void Bvh::shiftOrigin(const Vector3& newOrigin)
	{
		// Build array of leaves. Free the rest.
		for (i32 i = 0; i < m_nodeCapacity; ++i)
		{
			m_nodes[i].aabb.vMin -= newOrigin;
			m_nodes[i].aabb.vMax -= newOrigin;
		}
	}

	void* Bvh::getUserData(i32 proxyId) const
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].userData;
	}

	const AABB& Bvh::getFatAABB(i32 proxyId) const
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].aabb;
	}

	void Bvh::query(BvhCb* callback, const AABB& aabb) const
	{
		GrowableStack<i32, 256> stack;
		stack.push(m_root);

		while (stack.getCount() > 0)
		{
			i32 nodeId = stack.pop();
			if (nodeId == NullNode)
			{
				continue;
			}

			const BvhNode* node = m_nodes + nodeId;

			if (node->aabb.isIntersected(aabb))
			{
				if (node->IsLeaf())
				{
					bool proceed = callback->queryCallback(nodeId);
					if (proceed == false)
					{
						return;
					}
				}
				else
				{
					stack.push(node->child1);
					stack.push(node->child2);
				}
			}
		}
	}

	void Bvh::query(BvhCb* callback, const Frustum& frustum) const
	{
		GrowableStack<i32, 256> stack;
		stack.push(m_root);

		while (stack.getCount() > 0)
		{
			i32 nodeId = stack.pop();
			if (nodeId == NullNode)
			{
				continue;
			}

			const BvhNode* node = m_nodes + nodeId;
			if (frustum.isAABBIn(node->aabb.vMin, node->aabb.vMax))
			{
				if (node->IsLeaf())
				{
					bool proceed = callback->queryCallback(nodeId);
					if (proceed == false)
					{
						return;
					}
				}
				else
				{
					stack.push(node->child1);
					stack.push(node->child2);
				}
			}
		}
	}

	void Bvh::rayCast(BvhCb* callback, const Vector3& start, const Vector3& end) const
	{
		Vector3 p1 = start;
		Vector3 p2 = end;
		Vector3 r = p2 - p1;
		EchoAssert(r.len() > 0.0f);
		r.normalize();

		float maxFraction = 1.f;

		// Build a bounding box for the segment.
		AABB segmentAABB;
		segmentAABB.addPoint(start);
		segmentAABB.addPoint(end);

		GrowableStack<i32, 256> stack;
		stack.push(m_root);

		while (stack.getCount() > 0)
		{
			i32 nodeId = stack.pop();
			if (nodeId == NullNode)
			{
				continue;
			}

			const BvhNode* node = m_nodes + nodeId;

			if (!node->aabb.isIntersected(segmentAABB))
			{
				continue;
			}

			Ray ray(p1, r);
			if (!ray.hitBox(node->aabb))
			{
				continue;
			}

			if (node->IsLeaf())
			{
				//b2RayCastInput subInput;
				//subInput.p1 = input.p1;
				//subInput.p2 = input.p2;
				//subInput.maxFraction = maxFraction;

				float value = callback->rayCastCallback(nodeId);
				if (value == 0.0f)
				{
					// The client has terminated the ray cast.
					return;
				}

				if (value > 0.0f)
				{
					// Update segment bounding box.
					maxFraction = value;
					Vector3 t = p1 + maxFraction * (p2 - p1);
					Vector3::Min(segmentAABB.vMin, p1, t);
					Vector3::Max(segmentAABB.vMax, p1, t);
				}
			}
			else
			{
				stack.push(node->child1);
				stack.push(node->child2);
			}
		}
	}
}