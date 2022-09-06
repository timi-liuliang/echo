#include "ShaderEditor.h"
#include <nodeeditor/internal/node/Node.hpp>
#include <nodeeditor/NodeData>
#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include <nodeeditor/DataModelRegistry>
#include <nodeeditor/TypeConverter>
#include <engine/core/render/base/shader/editor/node/shader_node.h>
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

        Echo::StringArray classNames;
        Echo::Class::getChildClasses(classNames, "ShaderNode", true);
        for (const String& className : classNames)
        {
            if (!Echo::Class::isVirtual(className))
            {
				ShaderNode* shaderNode = dynamic_cast<ShaderNode*>(Class::create(className));
				if (shaderNode)
				{
					ret->registerModel(shaderNode->name(), shaderNode->category(), [className]()
					{
						ShaderNode* newNode = dynamic_cast<ShaderNode*>(Class::create(className));
						return std::unique_ptr<QtNodes::NodeDataModel>(newNode);
					});

					EchoSafeDelete(shaderNode, ShaderNode);
				}
            }
        }

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

        QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(replaceTemplate()));
        m_timer.start(200);
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
            shaderNode->generateCode(*m_shaderCompiler);
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

    Echo::String ShaderEditor::getTemplateName()
    {
        Echo::String templateName;
        if (m_shaderProgram->getDomain().getValue() == "Lighting")
        {
            templateName = "ShaderTemplateLighting";
        }
        else
        {
            if (m_shaderProgram->getBlendMode().getValue() == "Opaque")
            {
                templateName = "ShaderTemplateOpaque";
            }
            else
            {
                templateName = "ShaderTemplateTransparent";
            }
        }

        return templateName;
    }

    void ShaderEditor::replaceTemplate()
    {
        QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
        if (flowScene)
        {
            QtNodes::Node* shaderTemplateNode = m_graphicsScene->getShaderTemplateNode();
            if (shaderTemplateNode)
            {
                Echo::ShaderNodeTemplate* shaderTempate = dynamic_cast<Echo::ShaderNodeTemplate*>(shaderTemplateNode->nodeDataModel());
                if (shaderTempate && shaderTempate->name() != getTemplateName().c_str())
                {
                    Echo::Time::instance()->addDelayTask(200, [&]()
                        {
                            QPointF pos = shaderTemplateNode->nodeGraphicsObject().pos();

                            flowScene->nodeDeleted(*shaderTemplateNode);

                            std::unique_ptr<QtNodes::NodeDataModel> type = flowScene->registry().create(getTemplateName().c_str());
                            if (type)
                            {
                                auto& node = flowScene->createNode(std::move(type));
                                node.nodeGraphicsObject().setPos(QPointF(pos.x(), pos.y()));

                                flowScene->nodePlaced(node);
                            }
                        });
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
            Echo::ShaderNodeTemplate* shaderTemplateNode  = m_graphicsScene->getShaderTemplate();
            if (shaderTemplateNode)
            {
                m_shaderCompiler = shaderTemplateNode->getCompiler();
                m_shaderCompiler->reset();

                using namespace std::placeholders;
                flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorAllNodes, this, _1));

                if (m_shaderCompiler->compile())
                {
                    if (m_shaderCompiler->isValid() && m_shaderProgram)
                    {
                        Echo::String graph = flowScene->saveToMemory().toStdString().c_str();
                        m_shaderProgram->setGraph(graph);

                        m_shaderProgram->setVsCode(m_shaderCompiler->getVsCode());
                        m_shaderProgram->setPsCode(m_shaderCompiler->getPsCode());

                        flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorUniformDefaultValues, this, _1));
                    }
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
					Echo::Time::instance()->addDelayTask(200, [&]() 
                    {
						std::unique_ptr<QtNodes::NodeDataModel> type = flowScene->registry().create(getTemplateName().c_str());
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

    void ShaderEditor::save()
    {
        compile();
        
        if (m_shaderProgram)
        {
            m_shaderProgram->save();
        }
    }
}
