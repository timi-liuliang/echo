#pragma once

#include "editor_settings.h"

namespace Echo
{
    class EditorRenderSettings : public EditorSettings
    {
        ECHO_SINGLETON_CLASS(EditorRenderSettings, EditorSettings)
        
    public:
        EditorRenderSettings();
        virtual ~EditorRenderSettings();
        
        // instance
        static EditorRenderSettings* instance();

		// polygon mode
		const StringOption& getPolygonMode() const { return m_polygonMode; }
        void setPolygonMode(const StringOption& option);

    private:
		StringOption		m_polygonMode = StringOption("Fill", { "Fill","WireFrame" });
    };
}
