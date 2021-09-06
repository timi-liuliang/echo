#pragma once

#include "editor_settings.h"

namespace Echo
{
    class EditorGeneralSettings : public EditorSettings
    {
        ECHO_SINGLETON_CLASS(EditorGeneralSettings, EditorSettings)
        
    public:
        EditorGeneralSettings();
        virtual ~EditorGeneralSettings();
        
        // instance
        static EditorGeneralSettings* instance();

		// Show 3d grid
		void setShow3dGrid(bool show3dGrid) { m_show3dGrid = show3dGrid; }
		bool isShow3dGrid() const { return m_show3dGrid; }

    private:
        bool              m_show3dGrid = true;
    };
}
