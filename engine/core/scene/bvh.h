#pragma once

#include "engine/core/util/GrowableStack.h"
#include "engine/core/util/Any.hpp"
#include "engine/core/geom/AABB.h"
#include "engine/core/geom/Frustum.h"

namespace Echo
{
	class BvhCb
	{
	public:
		BvhCb() {}
		virtual ~BvhCb() {}

		// cb
		virtual bool queryCallback(i32 nodeId) = 0;
		virtual float rayCastCallback(i32 nodeId) = 0;
	};

	// node
	struct BvhNode
	{
		bool IsLeaf() const
		{
			return child1 == -1;
		}

		/// Enlarged AABB
		AABB aabb;

		any userData;

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
		i32 createProxy(const AABB& aabb, const any& userData);
		void destroyProxy(i32 proxyId);

		// Move a proxy with a swepted AABB. If the proxy has moved outside of its fattened AABB,
		// then the proxy is removed from the tree and re-inserted. Otherwise
		// the function returns immediately.
		// @return true if the proxy was re-inserted.
		bool moveProxy(i32 proxyId, const AABB& aabb1, const Vector3& displacement);

		// Get proxy user data.
		// @return the proxy user data or 0 if the id is invalid.
		const any& getUserData(i32 proxyId) const;

		// Get the fat AABB for a proxy.
		const AABB& getFatAABB(i32 proxyId) const;

		// Query an AABB for overlapping proxies. The callback class
		// is called for each proxy that overlaps the supplied AABB.
		void query(BvhCb* callback, const AABB& aabb) const;
		void query(BvhCb* callback, const Frustum& frustum) const;

		// Ray-cast against the proxies in the tree. This relies on the callback
		// to perform a exact ray-cast in the case were the proxy contains a shape.
		// The callback also performs the any collision filtering. This has performance
		// roughly equal to k * log(n), where k is the number of collisions and n is the
		// number of proxies in the tree.
		// @param input the ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
		// @param callback a callback class that is called for each proxy that is hit by the ray.
		void rayCast(BvhCb* callback, const Vector3& start, const Vector3& end) const;

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
}