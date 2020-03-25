#include "shader_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    ShaderEditor::ShaderEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    ShaderEditor::~ShaderEditor()
    {
    }
    
    // get camera2d icon, used for editor
    const char* ShaderEditor::getEditorIcon() const
    {
        return "engine/core/render/base/editor/icon/shader.png";
    }
#endif
}

