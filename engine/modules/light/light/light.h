#pragma once

#include <engine/core/scene/node.h>

namespace Echo
{
	class Light : public Node
	{
		ECHO_VIRTUAL_CLASS(Light, Node);

	public:
        virtual ~Light();
		Light();

		// 2d
		bool is2d() const { return m_2d; }
		void set2d(bool is2d) { m_2d = is2d; }

	protected:
		bool	m_2d = false;
	};
}
