#include "http_client_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	HttpClientEditor::HttpClientEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	HttpClientEditor::~HttpClientEditor()
	{
	}

	ImagePtr HttpClientEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/net/editor/icon/httpclient.png");
	}
#endif
}
