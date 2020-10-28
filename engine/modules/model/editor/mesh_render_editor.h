#pragma once

#include "particle_system_panel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
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
    
#endif
}
