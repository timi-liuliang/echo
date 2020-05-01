#pragma once

#include "../terrain.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TerrainEditor : public ObjectEditor
    {
    public:
        TerrainEditor(Object* object);
        virtual ~TerrainEditor();
        
        // get icon, used for editor
        virtual ImagePtr getThumbnail() const override;
        
    private:
    };
    
#endif
}
