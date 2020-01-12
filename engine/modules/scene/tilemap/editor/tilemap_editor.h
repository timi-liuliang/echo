#pragma once

#include "../tilemap.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TileMapEditor : public ObjectEditor
    {
    public:
        TileMapEditor(Object* object);
        virtual ~TileMapEditor();
        
        // get camera2d icon, used for editor
        const char* getEditorIcon() const;
        
    private:
    };
    
#endif
}
