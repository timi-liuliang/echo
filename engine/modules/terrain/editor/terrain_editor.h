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
        
        // get camera2d icon, used for editor
        const char* getEditorIcon() const;
        
    private:
    };
    
#endif
}
