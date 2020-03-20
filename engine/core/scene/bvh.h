#pragma once

#include "engine/core/geom/AABB.h"

namespace Echo
{
	// node
	struct BvhNode
	{
		bool IsLeaf() const
		{
			return child1 == -1;
		}

		/// Enlarged AABB
		AABB aabb;

		void* userData;

		union
		{
			i32 parent;
			i32 next;
		};

		i32 child1;
		i32 child2;

		// leaf = 0, free node = -1
		i32 height;
	};

	class Bvh
	{
	public:
		Bvh();
		~Bvh();

		// proxy
		i32 createProxy(const AABB& aabb, void* userData);
		void destroyProxy(i32 proxyId);

		// Move a proxy with a swepted AABB. If the proxy has moved outside of its fattened AABB,
		// then the proxy is removed from the tree and re-inserted. Otherwise
		// the function returns immediately.
		// @return true if the proxy was re-inserted.
		bool moveProxy(i32 proxyId, const AABB& aabb1, const Vector3& displacement);

		// Get proxy user data.
		// @return the proxy user data or 0 if the id is invalid.
		void* getUserData(i32 proxyId) const;

		/// Get the fat AABB for a proxy.
		const AABB& getFatAABB(i32 proxyId) const;

		/// Query an AABB for overlapping proxies. The callback class
		/// is called for each proxy that overlaps the supplied AABB.
		template <typename T> void query(T* callback, const AABB& aabb) const;

		/// Ray-cast against the proxies in the tree. This relies on the callback
		/// to perform a exact ray-cast in the case were the proxy contains a shape.
		/// The callback also performs the any collision filtering. This has performance
		/// roughly equal to k * log(n), where k is the number of collisions and n is the
		/// number of proxies in the tree.
		/// @param input the ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
		/// @param callback a callback class that is called for each proxy that is hit by the ray.
		template<typename T> void rayCast(T* callback, const Vector3& start, const Vector3& end) const;

		// Validate this tree. For testing.
		void validate() const;

		// Compute the height of the binary tree in O(N) time. Should not be called often.
		i32 getHeight() const;

		// Get the maximum balance of an node in the tree. The balance is the difference in height of the two children of a node.
		i32 getMaxBalance() const;

		/// Get the ratio of the sum of the node areas to the root area.
		float getAreaRatio() const;

		/// Build an optimal tree. Very expensive. For testing.
		void rebuildBottomUp();

		/// Shift the world origin. Useful for large worlds.
		/// The shift formula is: position -= newOrigin
		/// @param newOrigin the new origin with respect to the old origin
		void shiftOrigin(const Vector3& newOrigin);

	private:
		i32 allocateNode();
		void freeNode(i32 node);

		void insertLeaf(i32 node);
		void removeLeaf(i32 node);

		i32 balance(i32 index);

		i32 computeHeight() const;
		i32 computeHeight(i32 nodeId) const;

		void validateStructure(i32 index) const;
		void validateMetrics(i32 index) const;

	private:
		i32			m_root;
		BvhNode*	m_nodes;
		i32			m_nodeCount;
		i32			m_nodeCapacity;
		i32			m_freeList;
		ui32		m_path;				// This is used to incrementally traverse the tree for re-balancing.
		i32			m_insertionCount;
	};

	inline void* Bvh::getUserData(i32 proxyId) const
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].userData;
	}

	inline const AABB& Bvh::getFatAABB(i32 proxyId) const
	{
		EchoAssert(0 <= proxyId && proxyId < m_nodeCapacity);
		return m_nodes[proxyId].aabb;
	}

	template <typename T> inline void Bvh::query(T* callback, const AABB& aabb) const
	{
		b2GrowableStack<int32, 256> stack;
		stack.Push(m_root);

		while (stack.GetCount() > 0)
		{
			int32 nodeId = stack.Pop();
			if (nodeId == b2_nullNode)
			{
				continue;
			}

			const b2TreeNode* node = m_nodes + nodeId;

			if (b2TestOverlap(node->aabb, aabb))
			{
				if (node->IsLeaf())
				{
					bool proceed = callback->QueryCallback(nodeId);
					if (proceed == false)
					{
						return;
					}
				}
				else
				{
					stack.Push(node->child1);
					stack.Push(node->child2);
				}
			}
		}
	}

	template<typename T> inline void Bvh::rayCast(T* callback, const Vector3& start, const Vector3& end) const
	{
		Vector3 p1 = start;
		Vector3 p2 = end;
		Vector3 r = p2 - p1;
		EchoAssert(r.len() > 0.0f);
		r.normalize();

		// v is perpendicular to the segment.
		b2Vec2 v = b2Cross(1.0f, r);
		b2Vec2 abs_v = b2Abs(v);

		// Separating axis for segment (Gino, p80).
		// |dot(v, p1 - c)| > dot(|v|, h)

		float32 maxFraction = input.maxFraction;

		// Build a bounding box for the segment.
		b2AABB segmentAABB;
		{
			b2Vec2 t = p1 + maxFraction * (p2 - p1);
			segmentAABB.lowerBound = b2Min(p1, t);
			segmentAABB.upperBound = b2Max(p1, t);
		}

		b2GrowableStack<int32, 256> stack;
		stack.Push(m_root);

		while (stack.GetCount() > 0)
		{
			int32 nodeId = stack.Pop();
			if (nodeId == b2_nullNode)
			{
				continue;
			}

			const b2TreeNode* node = m_nodes + nodeId;

			if (b2TestOverlap(node->aabb, segmentAABB) == false)
			{
				continue;
			}

			// Separating axis for segment (Gino, p80).
			// |dot(v, p1 - c)| > dot(|v|, h)
			b2Vec2 c = node->aabb.GetCenter();
			b2Vec2 h = node->aabb.GetExtents();
			float32 separation = b2Abs(b2Dot(v, p1 - c)) - b2Dot(abs_v, h);
			if (separation > 0.0f)
			{
				continue;
			}

			if (node->IsLeaf())
			{
				b2RayCastInput subInput;
				subInput.p1 = input.p1;
				subInput.p2 = input.p2;
				subInput.maxFraction = maxFraction;

				float32 value = callback->RayCastCallback(subInput, nodeId);

				if (value == 0.0f)
				{
					// The client has terminated the ray cast.
					return;
				}

				if (value > 0.0f)
				{
					// Update segment bounding box.
					maxFraction = value;
					b2Vec2 t = p1 + maxFraction * (p2 - p1);
					segmentAABB.lowerBound = b2Min(p1, t);
					segmentAABB.upperBound = b2Max(p1, t);
				}
			}
			else
			{
				stack.Push(node->child1);
				stack.Push(node->child2);
			}
		}
	}
}