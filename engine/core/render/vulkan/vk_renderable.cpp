#include "vk_renderable.h"

namespace Echo
{
    VKRenderable::VKRenderable(const String& renderStage, ShaderProgramRes* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {
        
    }
}
