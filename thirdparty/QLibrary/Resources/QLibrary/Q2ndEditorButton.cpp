#include "Q2ndEditorButton.h"
#include "QPropertyModel.h"

namespace QT_UI
{
	Q2ndEditor::Q2ndEditor(QPropertyModel* model, QWidget* parent)
		: m_model(model), QPushButton( parent)
	{ 
		// ÏûÏ¢
		connect( this, SIGNAL(clicked()), this, SLOT(onClicked()));
	}

	Q2ndEditor::~Q2ndEditor()
	{

	}

	void Q2ndEditor::onClicked()
	{
		m_model->on2ndSubEditorCallback(text().toLocal8Bit().data()); 
	}
}