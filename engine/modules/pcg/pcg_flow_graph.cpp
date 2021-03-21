#include "pcg_flow_graph.h"
#include "engine/core/log/log.h"
#include "engine/core/main/Engine.h"
#include "engine/core/scene/node_tree.h"
#include <queue>
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>
#include <thirdparty/pugixml/pugixml_ext.hpp>

namespace Echo
{
	PCGFlowGraph::PCGFlowGraph()
	{
		setRenderType("3d");

		m_pgNode = EchoNew(PCGNode);
	}

	PCGFlowGraph::~PCGFlowGraph()
	{
		EchoSafeDelete(m_pgNode, PCGNode);
	}

	void PCGFlowGraph::bindMethods()
	{
		CLASS_BIND_METHOD(PCGFlowGraph, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(PCGFlowGraph, setMaterial, DEF_METHOD("setMaterial"));
		CLASS_BIND_METHOD(PCGFlowGraph, getPGContent, DEF_METHOD("getPGContent"));
		CLASS_BIND_METHOD(PCGFlowGraph, setPGContent, DEF_METHOD("setPGContent"));

		CLASS_REGISTER_PROPERTY(PCGFlowGraph, "Content", Variant::Type::Base64String, "getPGContent", "setPGContent");
		CLASS_REGISTER_PROPERTY(PCGFlowGraph, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY_HINT(PCGFlowGraph, "Material", PropertyHintType::ResourceType, "Material");
	}

	void PCGFlowGraph::setMesh(MeshPtr mesh)
	{ 
		m_mesh = mesh;
		m_localAABB = m_mesh ? m_mesh->getLocalBox() : AABB();

		m_isRenderableDirty = true;
	}

	void PCGFlowGraph::setMaterial(Object* material)
	{
		m_material = (Material*)material;

		m_isRenderableDirty = true;
	}

	const Base64String& PCGFlowGraph::getPGContent()
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

	void PCGFlowGraph::setPGContent(const Base64String& content)
	{ 
		m_pgContent = content;
		String origin = m_pgContent.decode();

		pugi::xml_document doc;
		doc.load_string(origin.c_str());

		pugi::xml_node root = doc.child("pg_geometry");
		if (root)
		{
			pugi::xml_node pgNodeXml = root.child("pg_node");
			PCGNode* pgNode = PCGNode::instanceNodeTree(&pgNodeXml, nullptr);
			if (pgNode)
			{
				EchoSafeDelete(m_pgNode, PCGNode);
				m_pgNode = pgNode;
			}

			pugi::xml_node connectNodeXml = root.append_child("pg_connects");
		}
	}

	void PCGFlowGraph::update_self()
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

	void PCGFlowGraph::buildRenderable()
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

	void PCGFlowGraph::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
	}

	bool PCGFlowGraph::isDirty()
	{
		std::queue<PCGNode*> nodes;
		nodes.push(m_pgNode);
		while (nodes.size())
		{
			PCGNode* node = nodes.front();
			if (node->isFinal() || !node->getParent())
			{
				if (node->isDirty())
					return true;

				for (PCGNode* child : node->children())
					nodes.push(child);
			}

			nodes.pop();
		}

		return false;
	}

	void PCGFlowGraph::run()
	{
		m_pgData.clear();


		setMesh(m_pgData.buildMesh());
	}
}
