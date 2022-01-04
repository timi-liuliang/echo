#pragma once

#include "../video_player.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class VideoCaptureEditor : public ObjectEditor
	{
	public:
		VideoCaptureEditor(Object* object);
		virtual ~VideoCaptureEditor();

		// get camera2d icon, used for editor
        virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
