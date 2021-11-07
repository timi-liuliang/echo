#include "texture_atla_editor.h"
#include "engine/core/render/base/texture/texture_atla.h"
#include "engine/core/render/base/texture/texture.h"
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
            ImagePtr image = Image::loadFromFile(atla->getTexture()->getPath());
            i32 left = atla->getViewport().x;
            i32 top = atla->getViewport().y;
            i32 width = atla->getViewport().z;
            i32 height = atla->getViewport().w;

            return image->getAtla(0, 0, left, top, width, height);
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

