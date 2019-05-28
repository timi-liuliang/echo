#include "event_region_rect_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    UiEventRegionRectEditor::UiEventRegionRectEditor(Object* object)
    : ObjectEditor(object)
    {
    }
    
    UiEventRegionRectEditor::~UiEventRegionRectEditor()
    {
    }
    
    // get camera2d icon, used for editor
    const char* UiEventRegionRectEditor::getEditorIcon() const
    {
        return "engine/modules/ui/editor/icon/event_region_rect.png";
    }
#endif
}
