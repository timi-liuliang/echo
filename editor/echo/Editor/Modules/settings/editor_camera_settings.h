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

        // near clip
        void setNearClip(float nearClip) { m_nearClip = nearClip; }
        float getNearClip() const { return m_nearClip; }
        
        // far clip
        void setFarClip(float farClip) { m_farClip = farClip; }
        float getFarClip() const { return m_farClip; }

    private:
		float		m_moveSpeed = 1.f;
        float       m_nearClip = 0.2f;
        float       m_farClip = 2500.f;
    };
}
