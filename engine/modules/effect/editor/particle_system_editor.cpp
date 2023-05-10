#include "particle_system_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    ParticleSystemEditor::ParticleSystemEditor(Object* object)
    : ObjectEditor(object)
    {
        m_panel = EchoNew(ParticleSystemPanel(object));
    }
    
    ParticleSystemEditor::~ParticleSystemEditor()
    {
        EchoSafeDelete(m_panel, ParticleSystemPanel);
    }

    ImagePtr ParticleSystemEditor::getThumbnail() const
    {
        return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/effect/editor/icon/particle_system.png");
    }

    void ParticleSystemEditor::onEditorSelectThisNode()
    {
        Editor::instance()->showBottomPanel(m_panel);
    }

    void ParticleSystemEditor::editor_update_self()
    {
        if (m_panel)
        {
            m_panel->update();
        }
    }
#endif
}

