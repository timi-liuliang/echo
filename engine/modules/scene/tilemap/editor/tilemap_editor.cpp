#include "tilemap_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"
#include "engine/core/main/GameSettings.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
    TileMapEditor::TileMapEditor(Object* object)
    : ObjectEditor(object)
    {
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("2d");
    }
    
    TileMapEditor::~TileMapEditor()
    {
        EchoSafeDelete(m_gizmo, Gizmos);
    }
    
    // get camera2d icon, used for editor
    const char* TileMapEditor::getEditorIcon() const
    {
        return "engine/modules/scene/tilemap/editor/icon/tilemap.png";
    }

	void TileMapEditor::editor_update_self()
	{
		if (m_isSelect)
		{
			TileMap* tileMap = ECHO_DOWN_CAST<TileMap*>(m_object);
			if (!Engine::instance()->getConfig().m_isGame && tileMap)
			{
				const Matrix4& worldMatrix = tileMap->getWorldMatrix();
				float totalWidth = tileMap->getWidth() * tileMap->getTileSize().y;
				float totalHeight = tileMap->getHeight() * tileMap->getTileSize().x;

				m_gizmo->clear();

				// draw rows
				for (i32 i = 0; i <= tileMap->getHeight(); i++)
				{
					float y = i * tileMap->getTileSize().x;

					Vector3 v0(0.f, y, 0.f);
					Vector3 v1(totalWidth, y, 0.f);

					m_gizmo->drawLine(tileMap->flip(v0) * worldMatrix, tileMap->flip(v1) * worldMatrix, Color(0.f, 0.f, 1.f, 0.65f));
				}

				// draw columns
				for (i32 i = 0; i <= tileMap->getWidth(); i++)
				{
					float x = i * tileMap->getTileSize().y;
					Vector3 v0( x, 0.f, 0.f);
					Vector3 v1( x, totalHeight, 0.f);

					m_gizmo->drawLine(tileMap->flip(v0) * worldMatrix, tileMap->flip(v1) * worldMatrix, Color(0.f, 0.f, 1.f, 0.65f));
				}

				m_gizmo->update(Engine::instance()->getFrameTime(), true);
			}
		}
	}
#endif
}

