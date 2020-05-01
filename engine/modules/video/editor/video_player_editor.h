#pragma once

#include "../video_player.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class VideoPlayerEditor : public ObjectEditor
	{
	public:
		VideoPlayerEditor(Object* object);
		virtual ~VideoPlayerEditor();

		// get camera2d icon, used for editor
        virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
