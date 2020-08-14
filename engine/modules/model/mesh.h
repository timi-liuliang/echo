#pragma once

#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	class Mesh : public Render
	{
		ECHO_CLASS(Mesh, Render);

	public:
		Mesh();
		virtual ~Mesh();

	private:
		Mesh*		m_mesh = nullptr;
	};
}
