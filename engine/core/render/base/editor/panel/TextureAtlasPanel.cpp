#include "TextureAtlasPanel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TextureAtlasPanel::TextureAtlasPanel(Object* obj)
	{
		m_textureAtlas = ECHO_DOWN_CAST<TextureAtlas*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/panel/TextureAtlasPanel.ui");

		QWidget* splitter = EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			EditorApi.qSplitterSetStretchFactor(splitter, 0, 0);
			EditorApi.qSplitterSetStretchFactor(splitter, 1, 1);
		}
	}

	void TextureAtlasPanel::update()
	{
	}
#endif
}
