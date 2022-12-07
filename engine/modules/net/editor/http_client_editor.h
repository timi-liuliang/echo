#pragma once

#include "../http/http_client.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class HttpClientEditor : public ObjectEditor
	{
	public:
		HttpClientEditor(Object* object);
		virtual ~HttpClientEditor();

		// get camera2d icon, used for editor
        virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
