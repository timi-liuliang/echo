#pragma once

#include "../base/image.h"

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
        
    private:
    };
    
#endif
}
