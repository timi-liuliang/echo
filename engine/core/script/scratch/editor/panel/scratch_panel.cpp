#include "scratch_panel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ScratchEditorPanel::ScratchEditorPanel(Object* obj)
	{
		m_ui = EditorApi.qLoadUi("engine/core/script/scratch/editor/panel/scratch_panel.ui");

		QSplitter* splitter = (QSplitter*)EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			splitter->setStretchFactor(0, 0);
			splitter->setStretchFactor(1, 1);
		}
	}

	void ScratchEditorPanel::update()
	{
	}
#endif
}
