#include "texture_atla_editor.h"
#include "engine/core/render/base/atla/TextureAtla.h"
#include "engine/core/render/base/Texture.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TextureAtlaEditor::TextureAtlaEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    TextureAtlaEditor::~TextureAtlaEditor()
    {
    }

    ImagePtr TextureAtlaEditor::getThumbnail() const
    {
        TextureAtla* atla = ECHO_DOWN_CAST<TextureAtla*>(m_object);
        if (atla && atla->getTexture())
        {
            return Image::loadFromFile(atla->getTexture()->getPath());
        }

        return nullptr;
    }

	void TextureAtlaEditor::onEditorSelectThisNode()
	{
	}

	void TextureAtlaEditor::editor_update_self()
	{

	}
#endif
}

