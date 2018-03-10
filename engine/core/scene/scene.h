#pragma once

#include "Node.h"

namespace Echo
{
	class scene
	{
	public:
		scene();
		virtual ~scene();

		// update
		void tick(float delta);

		Node* root() { return m_node; }

	public:
		void load(const String& fileName){}
		void save(const String& fileName) {}

	private:
		String		m_path;
		Node*		m_node;
	};
}

