#include "event_region_rect_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"
#include "../event/event_region_rect.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    UiEventRegionRectEditor::UiEventRegionRectEditor(Object* object)
    : ObjectEditor(object)
    {
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("ui");
    }
    
    UiEventRegionRectEditor::~UiEventRegionRectEditor()
    {
		EchoSafeDelete(m_gizmo, Gizmos);
    }
    
    // get camera2d icon, used for editor
    const char* UiEventRegionRectEditor::getEditorIcon() const
    {
        return "engine/modules/ui/editor/icon/event_region_rect.png";
    }

	void UiEventRegionRectEditor::editor_update_self()
	{
		UiEventRegion* eventRegion = ECHO_DOWN_CAST<UiEventRegion*>(m_object);
		if (eventRegion && eventRegion->isEnable())
		{
			if (!Engine::instance()->getConfig().m_isGame)
			{
				UiEventRegionRect* regionRect = ECHO_DOWN_CAST<UiEventRegionRect*>(m_object);
	
				Vector3 halfUp = Vector3::UNIT_Y * regionRect->getHeight() * 0.5f;
				Vector3 halfRight = Vector3::UNIT_X * regionRect->getWidth() * 0.5f;
				Vector3 center = regionRect->getWorldPosition();

				Vector3 v0 = center - halfRight + halfUp;
				Vector3 v1 = center - halfRight - halfUp;
				Vector3 v2 = center + halfRight - halfUp;
				Vector3 v3 = center + halfRight + halfUp;

				m_gizmo->clear();
				m_gizmo->drawLine(v0, v1, Color::BLUE);
				m_gizmo->drawLine(v1, v2, Color::BLUE);
				m_gizmo->drawLine(v2, v3, Color::BLUE);
				m_gizmo->drawLine(v3, v0, Color::BLUE);

				m_gizmo->update(Engine::instance()->getFrameTime(), true);
			}
		}
	}
#endif
}
