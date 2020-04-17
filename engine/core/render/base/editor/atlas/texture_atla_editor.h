#pragma once

#include "engine/core/render/base/ShaderProgram.h"
#include "TextureAtlasPanel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TextureAtlaEditor : public ObjectEditor
    {
    public:
        TextureAtlaEditor(Object* object);
        virtual ~TextureAtlaEditor();
       
        // get thumbnail
        virtual ImagePtr getThumbnail() const;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

        // update self
        void editor_update_self();
        
    private:
    };
    
#endif
}
