#include "shader_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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
    
    ImagePtr ShaderEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/shader.png");
    }
#endif
}

