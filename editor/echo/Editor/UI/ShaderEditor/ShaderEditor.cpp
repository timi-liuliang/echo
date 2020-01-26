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

using namespace ShaderEditor;

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

        // shader template
        ret->registerModel<ShaderTemplateDataModel>("");
        
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
        int a =10;
    }

    void ShaderEditor::compile()
    {
        QtNodes::FlowScene* flowScene = (QtNodes::FlowScene*)m_graphicsScene;
        if(flowScene)
        {
            using namespace std::placeholders;
            flowScene->iterateOverNodeDataDependentOrder(std::bind(&ShaderEditor::visitorAllNodes, this, _1));
        }
    }

    void ShaderEditor::save()
    {
        compile();
    }
}
