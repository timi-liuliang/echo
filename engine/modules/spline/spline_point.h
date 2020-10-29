#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class SplinePoint : public Node
	{
		ECHO_CLASS(SplinePoint, Node)

		friend class Spline;

	public:
		SplinePoint();
		virtual ~SplinePoint();

		// point id
		void setPointId(i32 pointId) { m_pointId = pointId; }
		i32 getPointId();

		// neighbor
		void addNeighbor(i32 pointId);
		void removeNeighbor(i32 pointId);

	protected:
		i32					m_pointId = 0;
		set<i32>::type		m_neighbors;
	};
}