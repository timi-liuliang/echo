#pragma once

#include "procedural_geometry_panel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ProceduralGeometryEditor : public ObjectEditor
    {
    public:
        ProceduralGeometryEditor(Object* object);
        virtual ~ProceduralGeometryEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        ProceduralGeometryPanel* m_panel = nullptr;
    };
    
#endif
}
