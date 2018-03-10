#pragma once

#include "Node.h"

namespace Echo
{
	class scene
	{
	public:
		scene();
		virtual ~scene();

		// ¸üÐÂ(Ãë)
		void tick(float delta);
	
	private:
		Node*		m_node;
	};
}

