#include "procedural_geometry.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <queue>

namespace Echo
{
	ProceduralGeometry::ProceduralGeometry()
	{
		setRenderType("3d");

		m_pgNode = new PGNode;
		m_pgNode->setFinal(true);
	}

	ProceduralGeometry::~ProceduralGeometry()
	{

	}

	void ProceduralGeometry::bindMethods()
	{
		CLASS_BIND_METHOD(ProceduralGeometry, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(ProceduralGeometry, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(ProceduralGeometry, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY_HINT(ProceduralGeometry, "Material", PropertyHintType::ResourceType, "Material");
	}

	void ProceduralGeometry::setMesh(MeshPtr mesh)
	{ 
		m_mesh = mesh;
		m_localAABB = m_mesh ? m_mesh->getLocalBox() : AABB();

		m_isRenderableDirty = true;
	}

	void ProceduralGeometry::setMaterial(Object* material)
	{
		m_material = (Material*)material;

		m_isRenderableDirty = true;
	}

	void ProceduralGeometry::update_self()
	{
	#ifdef ECHO_EDITOR_MODE
		if (isDirty())
			play();
	#endif

		if (isNeedRender())
		{
			buildRenderable();
			if (m_renderable)
				m_renderable->submitToRenderQueue();
		}
	}

	void ProceduralGeometry::buildRenderable()
	{
		if (m_isRenderableDirty)
		{
			clearRenderable();

			// make sure one material is valid
			if (!m_material)
			{
				ShaderProgramPtr shader = ShaderProgram::getDefault3D({ "HAS_NORMALS" });

				// material
				m_material = ECHO_CREATE_RES(Material);
				m_material->setShaderPath(shader->getPath());
			}

			if (m_mesh)
			{
				m_renderable = Renderable::create(m_mesh, m_material, this);
			}

			m_isRenderableDirty = false;
		}
	}

	void ProceduralGeometry::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
	}

	bool ProceduralGeometry::isDirty()
	{
		std::queue<PGNode*> nodes;
		nodes.push(m_pgNode);
		while (nodes.size())
		{
			PGNode* node = nodes.front();
			if (node->isFinal())
			{
				if (node->isDirty())
					return true;

				for (PGNode* child : node->children())
					nodes.push(child);
			}

			nodes.pop();
		}

		return false;
	}

	void ProceduralGeometry::play()
	{
		m_data.clear();

		if (m_pgNode)
			m_pgNode->play(m_data);

		setMesh(m_data.buildMesh());
	}
}
