#include "ShaderScene.h"
#include "ShaderEditor.h"

namespace DataFlowProgramming
{
	ShaderScene::ShaderScene(std::shared_ptr<QtNodes::DataModelRegistry> registry, QObject* parent)
		: FlowScene(registry, parent)
	{

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
}
