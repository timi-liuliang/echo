#include "particle_system_editor.h"
#include "engine/core/render/base/atla/TextureAtla.h"
#include "engine/core/render/base/Texture.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    ParticleSystemEditor::ParticleSystemEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    ParticleSystemEditor::~ParticleSystemEditor()
    {
    }

    ImagePtr ParticleSystemEditor::getThumbnail() const
    {
        return nullptr;
    }

    void ParticleSystemEditor::onEditorSelectThisNode()
    {
    }

    void ParticleSystemEditor::editor_update_self()
    {

    }
#endif
}

