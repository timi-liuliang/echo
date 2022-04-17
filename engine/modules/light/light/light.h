#pragma once

#include <engine/core/scene/node.h>

namespace Echo
{
	class Light : public Node
	{
		ECHO_VIRTUAL_CLASS(Light, Node);

	public:
		// Type
		enum Type
		{
			Direction = 1 << 0,
			Point = 1 << 1,
			Spot = 1 << 2,
		};

	public:
        virtual ~Light();
		Light();

		// 2d
		bool is2d() const { return m_2d; }
		void set2d(bool is2d) { m_2d = is2d; }

	public:
		// Gather lights
		static vector<Light*>::type gatherLights(i32 types);

	protected:
		// update self
		virtual void updateInternal(float elapsedTime) override;

	protected:
		bool		m_2d = false;
		i32			m_bvhNodeId = -1;
		class Bvh*	m_bvh = nullptr;
	};
}
