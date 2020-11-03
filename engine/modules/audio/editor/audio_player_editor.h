#pragma once

#include "../audio_player.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AudioPlayerEditor : public ObjectEditor
	{
	public:
		AudioPlayerEditor(Object* object);
		virtual ~AudioPlayerEditor();

		// get icon, used for editor
		virtual ImagePtr getThumbnail() const override;

		// post create this object
		virtual void postEditorCreateObject() override;

		// update self
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo = nullptr;
		TexturePtr	m_albedo;
	};

#endif
}
