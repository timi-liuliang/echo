#include "TextureAtlasSplitDialog.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/io/MemoryReader.h"
#include "TextureAtlasPackage.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TextureAtlasSplitDialog::TextureAtlasSplitDialog()
	{
		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/atlas/TextureAtlasSplitDialog.ui");
	}

	TextureAtlasSplitDialog::~TextureAtlasSplitDialog()
	{
	}

	void TextureAtlasSplitDialog::update()
	{
	}
#endif
}
