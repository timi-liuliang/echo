#include "cube_light_capture_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	CubeLightCaptureEditor::CubeLightCaptureEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));

		m_albedo = (Texture*)Echo::Res::get(Engine::instance()->getRootPath() + "engine/modules/light/editor/icon/ibl.png");
	}

	CubeLightCaptureEditor::~CubeLightCaptureEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr CubeLightCaptureEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/light/editor/icon/ibl.png");
	}

	void CubeLightCaptureEditor::onEditorSelectThisNode()
	{
	}

	void CubeLightCaptureEditor::editor_update_self()
	{
		m_gizmo->clear();

		CubeLightCapture* audioPlayer = ECHO_DOWN_CAST<CubeLightCapture*>(m_object);
		if (audioPlayer && m_albedo)
		{
			m_gizmo->setRenderType("3d");
			m_gizmo->drawSprite(audioPlayer->getWorldPosition(), Color::WHITE, 120.f, m_albedo, Gizmos::RenderFlags::FixedPixel);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

