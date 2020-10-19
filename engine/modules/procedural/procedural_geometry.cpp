#include "procedural_geometry.h"
#include "engine/core/log/Log.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <queue>
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include <thirdparty/pugixml/pugixml_ext.hpp>

namespace Echo
{
	ProceduralGeometry::ProceduralGeometry()
	{
		setRenderType("3d");

		m_pgNode = new PGNode;
	}

	ProceduralGeometry::~ProceduralGeometry()
	{
		EchoSafeDelete(m_pgNode, PGNode);
	}

	void ProceduralGeometry::bindMethods()
	{
		CLASS_BIND_METHOD(ProceduralGeometry, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(ProceduralGeometry, setMaterial, DEF_METHOD("setMaterial"));
		CLASS_BIND_METHOD(ProceduralGeometry, getPGContent, DEF_METHOD("getPGContent"));
		CLASS_BIND_METHOD(ProceduralGeometry, setPGContent, DEF_METHOD("setPGContent"));

		CLASS_REGISTER_PROPERTY(ProceduralGeometry, "Content", Variant::Type::Base64String, "getPGContent", "setPGContent");
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

	const Base64String& ProceduralGeometry::getPGContent()
	{ 
		pugi::xml_document doc;
		pugi::xml_node root = doc.append_child("pg_geometry");

		pugi::xml_node node = root.append_child("pg_node");
		m_pgNode->saveXml(&node, true);

		pugi::xml_node connect = root.append_child("pg_connects");

		std::string origin = pugi::get_doc_string(doc);
		m_pgContent.encode(origin.c_str());

		return m_pgContent; 
	}

	void ProceduralGeometry::setPGContent(const Base64String& content)
	{ 
		m_pgContent = content;
		String origin = m_pgContent.decode();

		pugi::xml_document doc;
		doc.load_string(origin.c_str());

		pugi::xml_node root = doc.child("pg_geometry");
		if (root)
		{
			pugi::xml_node pgNodeXml = root.child("pg_node");
			PGNode* pgNode = PGNode::instanceNodeTree(&pgNodeXml, nullptr);
			if (pgNode)
			{
				EchoSafeDelete(m_pgNode, PGNode);
				m_pgNode = pgNode;
			}

			pugi::xml_node connectNodeXml = root.append_child("pg_connects");
		}
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
			if (node->isFinal() || !node->getParent())
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
		m_pgData.clear();

		if (m_pgNode)
		{
			m_pgNode->play(m_pgData);
		}

		setMesh(m_pgData.buildMesh());
	}
}
