#pragma once

#include "engine/core/render/base/shader/shader_program.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ShaderEditor : public ObjectEditor
    {
    public:
        ShaderEditor(Object* object);
        virtual ~ShaderEditor();
        
        // get icon, used for editor
        ImagePtr getThumbnail() const;
        
    private:
    };
    
#endif
}
