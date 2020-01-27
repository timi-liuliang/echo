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
#include "engine/core/io/IO.h"

using namespace ShaderEditor;

static const char* g_textTemplate = R"(<?xml version = "1.0" encoding = "utf-8"?>
<Shader type="glsl">
<VS>
#version 450

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
</VS>
<PS>
#version 450

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

</PS>
<BlendState>
    <BlendEnable value = "true" />
    <SrcBlend value = "BF_SRC_ALPHA" />
    <DstBlend value = "BF_INV_SRC_ALPHA" />
</BlendState>
<RasterizerState>
    <CullMode value = "CULL_NONE" />
</RasterizerState>
<DepthStencilState>
    <DepthEnable value = "false" />
    <WriteDepth value = "false" />
</DepthStencilState>
<SamplerState>
    <BiLinearMirror>
        <MinFilter value = "FO_LINEAR" />
        <MagFilter value = "FO_LINEAR" />
        <MipFilter value = "FO_NONE" />
        <AddrUMode value = "AM_CLAMP" />
        <AddrVMode value = "AM_CLAMP" />
    </BiLinearMirror>
</SamplerState>
<Texture>
    <stage no = "0" sampler = "BiLinearMirror" />
</Texture>
</Shader>
)";

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

        // shader template
        ret->registerModel<ShaderTemplateDataModel>("Template");
        
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

		m_graphicsScene = new QtNodes::FlowScene(registerDataModels());
		m_graphicsView = new QtNodes::FlowView((QtNodes::FlowScene*)m_graphicsScene, dockWidgetContents);
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
        
        m_result = g_textTemplate;
        m_result = Echo::StringUtil::Replace(m_result, "${FS_SHADER_CODE}", m_shaderCode.c_str());
    }

    void ShaderEditor::compile()
    {
        QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
        if(flowScene)
        {
            m_paramCode.clear();
            m_shaderCode.clear();
            
            using namespace std::placeholders;
            flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorAllNodes, this, _1));
        }
    }

    void ShaderEditor::save()
    {
        compile();
        
        const char* content = m_result.c_str();
        if (content)
        {
            Echo::String fullPath = Echo::IO::instance()->convertResPathToFullPath("Res://temp.shader");
            std::ofstream f(fullPath.c_str());

            f << content;

            f.flush();
            f.close();
        }
    }
}
