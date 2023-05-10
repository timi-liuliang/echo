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
		setupUi(this);

		if (m_splitter)
		{
			m_splitter->setStretchFactor(0, 0);
			m_splitter->setStretchFactor(1, 1);
		}
	}

	void ScratchEditorPanel::update()
	{
	}
#endif
}
