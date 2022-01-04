#include "video_capture_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	VideoCaptureEditor::VideoCaptureEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	VideoCaptureEditor::~VideoCaptureEditor()
	{
	}

	ImagePtr VideoCaptureEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/video/editor/icon/videocapture.png");
	}
#endif
}
