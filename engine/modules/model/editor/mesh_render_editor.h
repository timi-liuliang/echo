#pragma once

#include "engine/core/editor/object_editor.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class MeshRenderEditor : public ObjectEditor
    {
    public:
        MeshRenderEditor(Object* object);
        virtual ~MeshRenderEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
    };
}


#endif
