#pragma once

#include "particle_system_panel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ParticleSystemEditor : public ObjectEditor
    {
    public:
        ParticleSystemEditor(Object* object);
        virtual ~ParticleSystemEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        ParticleSystemPanel* m_panel = nullptr;
    };
    
#endif
}
