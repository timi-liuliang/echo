#pragma once

#include "../ShaderProgram.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ShaderEditor : public ObjectEditor
    {
    public:
        ShaderEditor(Object* object);
        virtual ~ShaderEditor();
        
        // get icon, used for editor
        const char* getEditorIcon() const;
        
    private:
    };
    
#endif
}
