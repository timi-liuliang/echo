#include "direction_light_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	DirectionLightEditor::DirectionLightEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));

		m_albedo = (Texture*)Echo::Res::get(Engine::instance()->getRootPath() + "engine/modules/light/editor/icon/direction_light.png");
	}

	DirectionLightEditor::~DirectionLightEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr DirectionLightEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/light/editor/icon/direction_light.png");
	}

	void DirectionLightEditor::onEditorSelectThisNode()
	{
	}

	void DirectionLightEditor::editor_update_self()
	{
		m_gizmo->clear();

		Light* light = ECHO_DOWN_CAST<Light*>(m_object);
		if (light && m_albedo)
		{
			m_gizmo->setRenderType(light->is2d() ? "2d" : "3d");
			m_gizmo->drawSprite(light->getWorldPosition(), Color::WHITE, 120.f, m_albedo, Gizmos::RenderFlags::FixedPixel);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

