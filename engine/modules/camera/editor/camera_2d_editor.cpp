#include "camera_2d_editor.h"
#include "engine/core/main/Engine.h"
#include "engine/core/main/GameSettings.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	Camera2DEditor::Camera2DEditor(Object* object)
		: ObjectEditor(object)
		, m_isSelect(false)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName( StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->set2d(true);
	}

	Camera2DEditor::~Camera2DEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	// get camera2d icon, used for editor
	const char* Camera2DEditor::getEditorIcon() const
	{
		return "engine/modules/camera/editor/icon/camera2d.png";
	}

	void Camera2DEditor::editor_update_self()
	{
		if (m_isSelect)
		{
			Camera* camera = NodeTree::instance()->get2dCamera();
			if (camera && !Engine::instance()->getConfig().m_isGame)
			{
				Vector3 halfUp = camera->getUp() * Echo::GameSettings::instance()->getDesignHeight() * 0.5f;
				Vector3 halfRight = camera->getRight() * Echo::GameSettings::instance()->getDesignWidth() * 0.5f;
				Vector3 center = ECHO_DOWN_CAST<Camera2D*>(m_object)->getWorldPosition();

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