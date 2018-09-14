#include "../camera_2d.h"
#include "engine/core/main/Engine.h"
#include "engine/core/main/GameSettings.h"
#include "engine/core/scene/node_tree.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	struct EditorData
	{
		bool		m_isSelected;
	};

	// gizmo, used to render selected camera2d
	static Gizmos* g_camera2DGizmo = nullptr;

	// get camera2d icon, used for editor
	const char* Camera2D::getEditorIcon() const
	{
		return "engine/modules/camera/editor/icon/camera2d.png";
	}

	// on editor select this node
	void Camera2D::onEditorSelectThisNode()
	{
		if (!Engine::instance()->getConfig().m_isGame && !g_camera2DGizmo)
		{
			g_camera2DGizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
			g_camera2DGizmo->setName("Gizmos 2d camera");
			g_camera2DGizmo->set2d(true);
		}
	}

	void Camera2D::editor_update_self()
	{
		Camera* camera = NodeTree::instance()->get2dCamera();
		if (camera && !Engine::instance()->getConfig().m_isGame)
		{
			Vector3 halfUp = camera->getUp() * Echo::GameSettings::instance()->getDesignHeight() * 0.5f;
			Vector3 halfRight = camera->getRight() * Echo::GameSettings::instance()->getDesignWidth() * 0.5f;
			Vector3 center = getWorldPosition();
			
			Vector3 v0 = center - halfRight + halfUp;
			Vector3 v1 = center - halfRight - halfUp;
			Vector3 v2 = center + halfRight - halfUp;
			Vector3 v3 = center + halfRight + halfUp;

			g_camera2DGizmo->clear();
			g_camera2DGizmo->drawLine( v0, v1, Color::BLUE);
			g_camera2DGizmo->drawLine( v1, v2, Color::BLUE);
			g_camera2DGizmo->drawLine( v2, v3, Color::BLUE);
			g_camera2DGizmo->drawLine( v3, v0, Color::BLUE);

			g_camera2DGizmo->update(Engine::instance()->getFrameTime(), true);
		}		
	}
#endif
}