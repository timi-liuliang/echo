#pragma once

#include "../tilemap.h"
#include "engine/core/editor/object_editor.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    class TileMapEditor : public ObjectEditor
    {
    public:
        TileMapEditor(Object* object);
        virtual ~TileMapEditor();
        
        // get camera2d icon, used for editor
        virtual ImagePtr getThumbnail() const override;

		// on editor selected this node
		virtual void onEditorSelectThisNode() override { m_isSelect = true; }

		// on editor unselected this node
		virtual void onEditorUnSelectThisNode() override { m_isSelect = false; }

		// on editor update this
		virtual void editor_update_self() override;
        
    private:
		Gizmos*     m_gizmo = nullptr;
		bool		m_isSelect;
    };
    
#endif
}
