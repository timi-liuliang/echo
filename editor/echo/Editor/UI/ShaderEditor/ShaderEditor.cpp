#include "ShaderEditor.h"
#include <nodeeditor/internal/node/Node.hpp>
#include <nodeeditor/NodeData>
#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include <nodeeditor/DataModelRegistry>
#include <nodeeditor/TypeConverter>
#include "ShaderTemplateDataModel.h"
#include "VertexAttributeDataModel.h"
#include "FloatDataModel.h"
#include "Vector3DataModel.h"
#include "ColorDataModel.h"
#include "TextureDataModel.h"
#include "LayerBlendDataModel.h"
#include "SharedUniformDataModel.h"
#include "DataModel/Uniforms/Vector2DataModel.h"
#include "DataModel/Uniforms/Vector4DataModel .h"
#include "DataModel/Math/AdditionDataModel.h"
#include "DataModel/Math/SubstractionDataModel.h"
#include "DataModel/Math/MultiplicationDataModel.h"
#include "DataModel/Math/DivisionDataModel.h"
#include "DataModel/Math/Utils/SplitDataModel.h"
#include "DataModel/Math/Utils/CombineDataModel.h"
#include "DataModel/Math/Functions/DotProductDataModel.h"
#include "DataModel/Math/Functions/CrossProductDataModel.h"
#include "Data/DataFloat.h"
#include "Data/DataVector2.h"
#include "Data/DataVector3.h"
#include "Data/DataVector4.h"
#include "Data/Converter/Converters.h"
#include "ShaderScene.h"
#include "ShaderView.h"
#include "engine/core/io/IO.h"
#include "engine/core/util/Timer.h"

using namespace DataFlowProgramming;

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

        // shader template
        ret->registerModel<ShaderTemplateDataModel>("skip me");
        
        // variables
        ret->registerModel<SharedUniformDataModel>("Uniforms");
        ret->registerModel<FloatDataModel>("Uniforms");
        ret->registerModel<Vector2DataModel>("Uniforms");
        ret->registerModel<Vector3DataModel>("Uniforms");
        ret->registerModel<Vector4DataModel>("Uniforms");
        ret->registerModel<ColorDataModel>("Uniforms");
        ret->registerModel<TextureDataModel>("Uniforms");

        // Inputs
        ret->registerModel<VertexAttributeDataModel>("Inputs");
        ret->registerModel<LayerBlendDataModel>("Inputs");

        // Maths
        ret->registerModel<AdditionDataModel>("Math");
        ret->registerModel<SubstractionDataModel>("Math");
        ret->registerModel<MultiplicationDataModel>("Math");
        ret->registerModel<DivisionDataModel>("Math");

        // Math.Utils
        ret->registerModel<SplitDataModel>("Math.Utils");
        ret->registerModel<CombineDataModel>("Math.Utils");

        // Math.Functions
		ret->registerModel<DotProductDataModel>("Math.Functions");
		ret->registerModel<CrossProductDataModel>("Math.Functions");

        // Converts
        ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(), DataAny(nullptr).type()), QtNodes::TypeConverter{ FloatToAny() });
        //ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(), DataVector2(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector2() });
        //ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(), DataVector3(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector3() });
        //ret->registerTypeConverter(std::make_pair(DataFloat(nullptr, "").type(), DataVector4(nullptr, "").type()), QtNodes::TypeConverter{ FloatToVector4() });
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

    void ShaderEditor::visitorAllNodes(QtNodes::NodeDataModel* dataModel)
    {
        ShaderDataModel* shaderDataModel = dynamic_cast<ShaderDataModel*>(dataModel);
        if(shaderDataModel)
        {
            shaderDataModel->generateCode(m_shaderCompiler);
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
					Echo::Time::instance()->addDelayTask(200, [flowScene, flowView]() {
						std::unique_ptr<NodeDataModel> type = flowScene->registry().create("ShaderTemplate");
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
