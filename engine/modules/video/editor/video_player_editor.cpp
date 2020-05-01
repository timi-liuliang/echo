#include "video_player_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	VideoPlayerEditor::VideoPlayerEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	VideoPlayerEditor::~VideoPlayerEditor()
	{
	}

	ImagePtr VideoPlayerEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/video/editor/icon/videoplayer.png");
	}
#endif
}
