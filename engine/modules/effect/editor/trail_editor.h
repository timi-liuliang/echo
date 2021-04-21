#pragma once

#include "engine/core/editor/object_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TrailEditor : public ObjectEditor
    {
    public:
        TrailEditor(Object* object);
        virtual ~TrailEditor();

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
