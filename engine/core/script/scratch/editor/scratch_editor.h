#pragma once

#include "engine/core/script/scratch/scratch.h"
#include "panel/scratch_panel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class ScratchEditor : public ObjectEditor
    {
    public:
        ScratchEditor(Object* object);
        virtual ~ScratchEditor();
       
		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        ScratchEditorPanel* m_panel = nullptr;
    };
    
#endif
}
