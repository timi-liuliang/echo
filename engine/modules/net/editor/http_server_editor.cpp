#include "http_server_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	HttpServerEditor::HttpServerEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	HttpServerEditor::~HttpServerEditor()
	{
	}

	ImagePtr HttpServerEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/net/editor/icon/httpserver.png");
	}
#endif
}
