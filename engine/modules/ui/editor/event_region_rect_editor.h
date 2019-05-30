#pragma once

#include "../base/image.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class UiEventRegionRectEditor : public ObjectEditor
    {
    public:
        UiEventRegionRectEditor(Object* object);
        virtual ~UiEventRegionRectEditor();
        
        // get camera2d icon, used for editor
        const char* getEditorIcon() const;

		//// on editor select unselect this node
		//virtual void onEditorSelectThisNode() override { m_isSelect = true; }
		//virtual void onEditorUnSelectThisNode() override { m_isSelect = false; }

		// on editor update this
		virtual void editor_update_self() override;
        
    private:
		Gizmos*		m_gizmo = nullptr;
		//bool		m_isSelect = false;
    };
    
#endif
}
