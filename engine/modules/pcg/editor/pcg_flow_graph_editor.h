#pragma once

#include "pcg_flow_graph_panel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class PCGFlowGraphEditor : public ObjectEditor
    {
    public:
        PCGFlowGraphEditor(Object* object);
        virtual ~PCGFlowGraphEditor();

        // get thumbnail
        virtual ImagePtr getThumbnail() const override;

        // on editor select this node
        virtual void onEditorSelectThisNode() override;

        // update self
        virtual void editor_update_self() override;
        
    private:
        PCGFlowGraphPanel* m_panel = nullptr;
    };
    
#endif
}
