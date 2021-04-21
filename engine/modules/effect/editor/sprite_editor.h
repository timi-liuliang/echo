#pragma once

#include "engine/core/editor/object_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class SpriteEditor : public ObjectEditor
    {
    public:
        SpriteEditor(Object* object);
        virtual ~SpriteEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
    };
    
#endif
}
