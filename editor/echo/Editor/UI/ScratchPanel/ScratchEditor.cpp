#include "ScratchEditor.h"

namespace Studio
{
	ScratchEditor::ScratchEditor(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi(this);
	}

	ScratchEditor::~ScratchEditor()
	{
	}
}
