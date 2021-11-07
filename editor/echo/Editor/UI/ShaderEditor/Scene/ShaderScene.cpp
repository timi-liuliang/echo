#include "ShaderScene.h"
#include "ShaderEditor.h"
#include <engine/core/render/base/shader/editor/node/template/shader_node_template.h>
#include "nodeeditor/internal/node/Node.hpp"

namespace DataFlowProgramming
{
	ShaderScene::ShaderScene(std::shared_ptr<QtNodes::DataModelRegistry> registry, QObject* parent)
		: FlowScene(registry, parent)
	{
		QObject::connect(this, &FlowScene::nodeContextMenu, this, &ShaderScene::onShowShaderNodeMenu);
	}

	ShaderScene::ShaderScene(QObject* parent)
		: FlowScene(parent)
	{

	}

	ShaderScene::~ShaderScene()
	{

	}

	void ShaderScene::compile()
	{
		if (m_shaderEditor)
		{
			m_shaderEditor->compile();
		}
	}

	void ShaderScene::removeNode(QtNodes::Node& node)
	{
		// Can't delete ShaderTemplate
		Echo::ShaderNodeTemplate* shaderTempateNode = dynamic_cast<Echo::ShaderNodeTemplate*>(node.nodeDataModel());
		if (!shaderTempateNode)
		{
			FlowScene::removeNode(node);
		}
	}

	QtNodes::Node* ShaderScene::getShaderTemplateNode()
	{
		for (QtNodes::Node* node : allNodes())
		{
			// Can't delete ShaderTemplate
			Echo::ShaderNodeTemplate* shaderTempateNode = dynamic_cast<Echo::ShaderNodeTemplate*>(node->nodeDataModel());
			if (shaderTempateNode)
			{
				return node;
			}
		}

		return nullptr;
	}

	void ShaderScene::onShowShaderNodeMenu(QtNodes::Node& node, const QPointF& pos)
	{
	}

	void ShaderScene::nodePressed(QtNodes::Node& node)
	{
		Echo::ShaderNode* shaderNode = dynamic_cast<Echo::ShaderNode*>(node.nodeDataModel());
		if (shaderNode)
		{
			if (!shaderNode->onNodePressed())
				m_shaderEditor->showShaderProgramProperties();
		}
	}
}
