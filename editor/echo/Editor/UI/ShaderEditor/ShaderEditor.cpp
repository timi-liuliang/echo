#include "ShaderEditor.h"
#include <nodeeditor/internal/node/Node.hpp>
#include <nodeeditor/NodeData>
#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include <nodeeditor/DataModelRegistry>
#include <nodeeditor/TypeConverter>
#include <engine/core/render/base/editor/shader/node/shader_node.h>
#include "ShaderScene.h"
#include "ShaderView.h"
#include "engine/core/io/IO.h"
#include "engine/core/util/Timer.h"
#include "NodeTreePanel.h"

using namespace DataFlowProgramming;
using namespace Echo;

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

  //      // shader template
  //      ret->registerModel<ShaderTemplateDataModel>("skip me");
  //      
  //      // variables
  //      ret->registerModel<SharedUniformDataModel>("Uniforms");
  //      ret->registerModel<FloatDataModel>("Uniforms");
  //      ret->registerModel<Vector2DataModel>("Uniforms");
  //      ret->registerModel<Vector3DataModel>("Uniforms");
  //      ret->registerModel<Vector4DataModel>("Uniforms");
  //      ret->registerModel<ColorDataModel>("Uniforms");
  //      ret->registerModel<ShaderNodeTexture>("Uniforms");

  //      // Inputs
  //      ret->registerModel<VertexAttributeDataModel>("Inputs");
  //      ret->registerModel<LayerBlendDataModel>("Inputs");

  //      // Maths
  //      ret->registerModel<AbsDataModel>("Math");
  //      ret->registerModel<AdditionDataModel>("Math");
  //      ret->registerModel<SubstractionDataModel>("Math");
  //      ret->registerModel<MultiplicationDataModel>("Math");
  //      ret->registerModel<DivisionDataModel>("Math");
		//ret->registerModel<DotProductDataModel>("Math");
		//ret->registerModel<CrossProductDataModel>("Math");
		//ret->registerModel<MinDataModel>("Math");
		//ret->registerModel<MaxDataModel>("Math");
		//ret->registerModel<PowDataModel>("Math");
		//ret->registerModel<SinDataModel>("Math");
		//ret->registerModel<CosDataModel>("Math");
		//ret->registerModel<ModDataModel>("Math");
		//ret->registerModel<FloorDataModel>("Math");
  //      ret->registerModel<FractDataModel>("Math");
  //      ret->registerModel<FwidthDataModel>("Math");
  //      ret->registerModel<LengthDataModel>("Math");
		//ret->registerModel<MixDataModel>("Math");
  //      ret->registerModel<SmoothStepDataModel>("Math");
  //      ret->registerModel<Echo::ShaderNodeATan2>("Math");
  //      ret->registerModel<Echo::ShaderNodeSign>("Math");

  //      // Utils
		//ret->registerModel<SplitDataModel>("Utils");
		//ret->registerModel<CombineDataModel>("Utils");
  //      ret->registerModel<Echo::ShaderNodeTextureSize>("Utils");

  //      // Custom
  //      ret->registerModel<Echo::ShaderNodeGLSL>("Custom");
  //      ret->registerModel<Echo::ShaderNodeGaussianBlur>("Blur");
  //      ret->registerModel<Echo::ShaderNodeZoomBlur>("Blur");
  //      ret->registerModel<Echo::ShaderNodeSpinBlur>("Blur");

  //      // Color
  //      ret->registerModel<Echo::ShaderNodeGrayScale>("Color");
  //      ret->registerModel<Echo::ShaderNodeSRgbToLinear>("Color");
  //      ret->registerModel<Echo::ShaderNodeLinearToSRgb>("Color");

        // Converts
        ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(),   DataVector2(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector2() });
        ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(),   DataVector3(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector3() });
        ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(),   DataVector4(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector4() });
        ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(),   DataAny(nullptr).type()), QtNodes::TypeConverter{ FloatToAny() });
        ret->registerTypeConverter(std::make_pair(DataVector2(nullptr, "").type(), DataAny(nullptr).type()), QtNodes::TypeConverter{ Vector2ToAny() });
        ret->registerTypeConverter(std::make_pair(DataVector3(nullptr, "").type(), DataAny(nullptr).type()), QtNodes::TypeConverter{ Vector3ToAny() });
        ret->registerTypeConverter(std::make_pair(DataVector4(nullptr, "").type(), DataAny(nullptr).type()), QtNodes::TypeConverter{ Vector4ToAny() });

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

    void ShaderEditor::showShaderProgramProperties()
    {
        Studio::NodeTreePanel::instance()->onEditObject(m_shaderProgram);
    }

    void ShaderEditor::visitorAllNodes(QtNodes::NodeDataModel* dataModel)
    {
		ShaderNode* shaderNode = dynamic_cast<ShaderNode*>(dataModel);
		if (shaderNode)
		{
            shaderNode->checkValidation();
            shaderNode->generateCode(m_shaderCompiler);
		}
    }

    void ShaderEditor::visitorUniformDefaultValues(QtNodes::NodeDataModel* dataModel)
    {
        Echo::ShaderNode* shaderNode = dynamic_cast<Echo::ShaderNode*>(dataModel);
        if (shaderNode)
        {
			Echo::StringArray uniformNames;
			Echo::VariantArray uniformValues;
			if (shaderNode->getDefaultValue(uniformNames, uniformValues))
			{
				for (size_t i = 0; i < uniformNames.size(); i++)
				{
					m_shaderProgram->setPropertyValue(uniformNames[i], uniformValues[i]);
				}
			}
        }
    }

    void ShaderEditor::compile()
    {
        if (m_isLoading)
            return;

        QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
        if(flowScene)
        {
            m_shaderCompiler.reset();
            
            using namespace std::placeholders;
            flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorAllNodes, this, _1));

            if (m_shaderCompiler.compile())
            {
				if (m_shaderCompiler.isValid() && m_shaderProgram)
				{
					Echo::String graph = flowScene->saveToMemory().toStdString().c_str();
					m_shaderProgram->setGraph(graph);

					m_shaderProgram->setVsCode(m_shaderCompiler.getVsCode());
					m_shaderProgram->setPsCode(m_shaderCompiler.getPsCode());

                    flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorUniformDefaultValues, this, _1));
				}
            }
        }
    }

    void ShaderEditor::open(const Echo::String& resPath)
    {
        m_isLoading = true;

        m_shaderProgram = dynamic_cast<Echo::ShaderProgram*>(Echo::Res::get(resPath));
        if(m_shaderProgram)
        {
            QtNodes::FlowScene* flowScene = m_graphicsScene;
            if(flowScene)
            {
                flowScene->clearScene();

                Echo::String graph = m_shaderProgram->getGraph();
                if (!graph.empty())
                {
                    flowScene->loadFromMemory(graph.c_str());

                    this->setVisible(true);
                }
                else
                {
                    this->setVisible(true);

                    // Create one ShaderTemplate node
					QtNodes::FlowView* flowView = m_graphicsView;
					Echo::Time::instance()->addDelayTask(200, [flowScene, flowView]() 
                    {
						std::unique_ptr<QtNodes::NodeDataModel> type = flowScene->registry().create("ShaderTemplate");
						if (type)
						{
                            QRectF sceneRect = flowView->sceneRect();

							auto& node = flowScene->createNode(std::move(type));
							QPoint pos(node.nodeGraphicsObject().x(), sceneRect.center().y() - node.nodeGraphicsObject().boundingRect().height() * 0.5f);

							node.nodeGraphicsObject().setPos(QPointF(pos.x(), pos.y()));
							flowScene->nodePlaced(node);
						}
					});
                }
            }
        }

        m_isLoading = false;
    }

    void ShaderEditor::adjustViewRect()
    {
   //     QtNodes::Node* shaderTemplateNode = m_graphicsScene->getShaderTemplateNode();
   //     if (shaderTemplateNode)
   //     {
   //         QRectF stRect = shaderTemplateNode->nodeGraphicsObject().sceneBoundingRect();

			//QRectF viewRect = m_graphicsView->sceneRect();
			//m_graphicsView->centerOn(&shaderTemplateNode->nodeGraphicsObject());
   //     }
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
