#include "ShaderEditor.h"
#include <nodeeditor/NodeData>
#include <nodeeditor/FlowScene>
#include <nodeeditor/FlowView>
#include <nodeeditor/DataModelRegistry>
#include "ShaderTemplateDataModel.h"
#include "FloatDataModel.h"
#include "Vector3DataModel.h"

using namespace ShaderEditor;

namespace Studio
{
	static std::shared_ptr<QtNodes::DataModelRegistry> registerDataModels()
	{
		auto ret = std::make_shared<QtNodes::DataModelRegistry>();

        ret->registerModel<FloatDataModel>("Variable");
        ret->registerModel<Vector3DataModel>("Variable");
        ret->registerModel<ShaderTemplateDataModel>("Template");
		//ret->registerModel<TextSourceDataModel>();
		//ret->registerModel<TextDisplayDataModel>();

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
}
