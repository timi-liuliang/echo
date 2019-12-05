#pragma once

#include "editor_settings.h"

namespace Echo
{
    class EditorCameraSettings : public EditorSettings
    {
        ECHO_SINGLETON_CLASS(EditorCameraSettings, EditorSettings)
        
    public:
		EditorCameraSettings();
        virtual ~EditorCameraSettings();
        
        // instance
        static EditorCameraSettings* instance();

		// move speed
		void setMoveSpeed(float moveSpeed);
		float getMoveSpeed() const { return m_moveSpeed; }
        
    private:
		float		m_moveSpeed = 1.f;
    };
}
