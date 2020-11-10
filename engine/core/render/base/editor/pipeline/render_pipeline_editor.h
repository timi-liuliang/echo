#pragma once

#include "engine/core/render/base/pipeline/render_pipeline.h"
#include "RenderPipelinePanel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class RenderPipelineEditor : public ObjectEditor
    {
    public:
        RenderPipelineEditor(Object* object);
        virtual ~RenderPipelineEditor();
        
		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// post process
		virtual void postEditorCreateObject() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        RenderpipelinePanel* m_panel = nullptr;
    };
    
#endif
}
