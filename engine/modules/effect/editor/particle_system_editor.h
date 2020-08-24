#pragma once

#include "../particle_system.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ParticleSystemEditor : public ObjectEditor
    {
    public:
        ParticleSystemEditor(Object* object);
        virtual ~ParticleSystemEditor();

        // is thumbnail static (every instance has same thumbnail)
        virtual bool isThumbnailStatic() const override { return false; }
       
        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
    };
    
#endif
}
