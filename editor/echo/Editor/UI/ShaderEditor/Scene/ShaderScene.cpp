#include "ShaderScene.h"
#include "ShaderEditor.h"
#include "ShaderTemplateDataModel.h"
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
		ShaderTemplateDataModel* shaderTempateNode = qobject_cast<ShaderTemplateDataModel*>(node.nodeDataModel());
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
			ShaderTemplateDataModel* shaderTempateNode = qobject_cast<ShaderTemplateDataModel*>(node->nodeDataModel());
			if (shaderTempateNode)
			{
				return node;
			}
		}

		return nullptr;
	}

	void ShaderScene::onShowShaderNodeMenu(QtNodes::Node& node, const QPointF& pos)
	{
		ShaderDataModel* shaderDataModel = qobject_cast<ShaderDataModel*>(node.nodeDataModel());
		if (shaderDataModel)
		{
			shaderDataModel->showMenu(pos);
		}
	}
}
