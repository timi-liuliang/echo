#pragma once

#include "engine/core/render/base/ShaderProgram.h"
#include "TextureAtlasPanel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TextureAtlasEditor : public ObjectEditor
    {
    public:
        TextureAtlasEditor(Object* object);
        virtual ~TextureAtlasEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        TextureAtlasPanel* m_panel = nullptr;
    };
    
#endif
}
