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

		// is thumbnail static (every instance has same thumbnail)
		virtual bool isThumbnailStatic() const { return false; }
       
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
