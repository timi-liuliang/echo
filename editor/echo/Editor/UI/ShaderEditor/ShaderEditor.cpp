#include "ShaderEditor.h"
#include <nodeeditor/NodeData>
#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include <nodeeditor/DataModelRegistry>
#include "ShaderTemplateDataModel.h"
#include "FloatDataModel.h"
#include "Vector3DataModel.h"
#include "ColorDataModel.h"
#include "TextureDataModel.h"
#include "ShaderScene.h"
#include "ShaderView.h"
#include "engine/core/io/IO.h"

using namespace DataFlowProgramming;

static const char* g_VsTemplate = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

// outputs
layout(location = 0) out vec2 v_TexCoord;

void main(void)
{
    vec4 position = vs_ubo.u_WorldViewProjMatrix * vec4(a_Position, 1.0);
    gl_Position = position;
    
    v_TexCoord = a_UV;
}
)";

static const char* g_PsTemplate = R"(#version 450

precision mediump float;

// uniforms
layout(binding = 0) uniform UBO
{
    float u_UiAlpha;
} fs_ubo;

// inputs
layout(location = 0) in vec2  v_TexCoord;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
${FS_SHADER_CODE}

    o_FragColor = vec4(__BaseColor.rgb, 1.0);
}
)";

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

        // shader template
        ret->registerModel<ShaderTemplateDataModel>("skip me");
        
        // variables
        ret->registerModel<FloatDataModel>("Variable");
        ret->registerModel<Vector3DataModel>("Variable");
        ret->registerModel<ColorDataModel>("Variable");
        ret->registerModel<TextureDataModel>("Variable");

		return ret;
	}

	ShaderEditor::ShaderEditor(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi(this);

		m_graphicsScene = new DataFlowProgramming::ShaderScene(registerDataModels());
        ((DataFlowProgramming::ShaderScene*)m_graphicsScene)->setShaderEditor(this);

		m_graphicsView = new DataFlowProgramming::ShaderView((QtNodes::FlowScene*)m_graphicsScene, dockWidgetContents);
		m_graphicsView->setFrameShape(QFrame::NoFrame);
		verticalLayout->addWidget(m_graphicsView);
	}

	ShaderEditor::~ShaderEditor()
	{
	}

    void ShaderEditor::visitorAllNodes(QtNodes::NodeDataModel* dataModel)
    {
        ShaderDataModel* shaderDataModel = dynamic_cast<ShaderDataModel*>(dataModel);
        if(shaderDataModel)
        {
            shaderDataModel->generateCode(m_paramCode, m_shaderCode);
        }
    }

    void ShaderEditor::compile()
    {
        if (m_isLoading)
            return;

        QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
        if(flowScene)
        {
            m_paramCode.clear();
            m_shaderCode.clear();
            
            using namespace std::placeholders;
            flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorAllNodes, this, _1));
            
            Echo::String vsCode = g_VsTemplate;
            
            Echo::String psCode = g_PsTemplate;
            psCode = Echo::StringUtil::Replace(psCode, "${FS_SHADER_CODE}", m_shaderCode.c_str());
            
            // remember graph
            if(m_shaderProgram)
            {
                Echo::String graph = flowScene->saveToMemory().toStdString().c_str();
                m_shaderProgram->setGraph(graph);
                
                m_shaderProgram->setVsCode(vsCode);
                
                m_shaderProgram->setPsCode(psCode);
            }
        }
    }

    void ShaderEditor::open(const Echo::String& resPath)
    {
        m_isLoading = true;

        m_shaderProgram = dynamic_cast<Echo::ShaderProgram*>(Echo::Res::get(resPath));
        if(m_shaderProgram)
        {
            QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
            if(flowScene)
            {
                flowScene->clearScene();

                Echo::String graph = m_shaderProgram->getGraph();
                if (!graph.empty())
                {
                    flowScene->loadFromMemory(graph.c_str());
                }
                else
                {
                    // Create one ShaderTemplate node
					std::unique_ptr<NodeDataModel> type = flowScene->registry().create("ShaderTemplate");
                    if (type)
                    {
                        auto& node = flowScene->createNode(std::move(type));
                        flowScene->nodePlaced(node);
                    }
                }

                m_graphicsView->centerOn(m_graphicsView->sceneRect().center());
            }
        }
        
        this->setVisible(true);

        m_isLoading = false;
    }

    void ShaderEditor::save()
    {
        compile();
        
        if (m_shaderProgram)
        {
            m_shaderProgram->save();
        }
    }
}
