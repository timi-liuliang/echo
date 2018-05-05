#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"
#include "engine/core/render/render/Renderable.h"

namespace Echo
{
	class GltfScene : public Node
	{
		ECHO_CLASS(GltfScene, Node)

	public:
		GltfScene();
		virtual ~GltfScene();

		// bind class methods to script
		static void bindMethods();

		// set texture res path
		void setGltfRes(const ResourcePath& path);

		// get texture res
		const ResourcePath& getGltfRes() { return m_gltfRes; }

	protected:
		// update
		virtual void update();

	private:
		ResourcePath			m_gltfRes;
	};
}