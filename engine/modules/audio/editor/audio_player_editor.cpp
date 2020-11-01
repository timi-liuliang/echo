#include "audio_player_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	AudioPlayerEditor::AudioPlayerEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("3d");

		//Echo::String resPath;
		//if (IO::instance()->convertFullPathToResPath(Engine::instance()->getRootPath() + "engine/modules/audio/editor/icon/audioplayer.png", resPath))
		{
			m_albedo = (Texture*)Echo::Res::get(Engine::instance()->getRootPath() + "engine/modules/audio/editor/icon/audioplayer.png");
		}
	}

	AudioPlayerEditor::~AudioPlayerEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr AudioPlayerEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/audio/editor/icon/audioplayer.png");
	}

	void AudioPlayerEditor::editor_update_self()
	{
		m_gizmo->clear();

		AudioPlayer* audioPlayer = ECHO_DOWN_CAST<AudioPlayer*>(m_object);
		if (audioPlayer && m_albedo)
		{
			m_gizmo->drawSprite(audioPlayer->getWorldPosition(), Color::WHITE, 120.f, m_albedo, Gizmos::RenderFlags::FixedPixel);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}
