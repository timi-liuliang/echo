#pragma once

#include "../ShaderProgram.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TextureAtlasEditor : public ObjectEditor
    {
    public:
        TextureAtlasEditor(Object* object);
        virtual ~TextureAtlasEditor();
        
        // get icon, used for editor
        const char* getEditorIcon() const;
        
    private:
    };
    
#endif
}
