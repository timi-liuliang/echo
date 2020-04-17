#include "texture_atlas_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TextureAtlasEditor::TextureAtlasEditor(Object* object)
    : ObjectEditor(object)
    {
        m_panel = EchoNew(TextureAtlasPanel(object));
    }
    
    TextureAtlasEditor::~TextureAtlasEditor()
    {
        EchoSafeDelete(m_panel, TextureAtlasPanel);
    }
    
    const char* TextureAtlasEditor::getEditorIcon() const
    {
        return "engine/core/render/base/editor/icon/texture_atlas.png";
    }

    ImagePtr TextureAtlasEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/texture_atlas.png");
    }

	void TextureAtlasEditor::onEditorSelectThisNode()
	{
		Editor::instance()->showBottomPanel(m_panel);
	}

	void TextureAtlasEditor::editor_update_self()
	{
		if (m_panel)
		{
			m_panel->update();
		}
	}
#endif
}

