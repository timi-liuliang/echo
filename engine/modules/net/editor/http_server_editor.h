#pragma once

#include "../http_server.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class HttpServerEditor : public ObjectEditor
	{
	public:
		HttpServerEditor(Object* object);
		virtual ~HttpServerEditor();

		// get camera2d icon, used for editor
        virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
