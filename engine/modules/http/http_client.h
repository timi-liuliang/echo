#pragma once

#include "engine/core/scene/node.h"
#include <cpp-httplib/httplib.h>

namespace Echo
{
	class HttpClient : public Node
	{
		ECHO_CLASS(HttpClient, Node)

	public:
		HttpClient();
		virtual ~HttpClient();

		// Get
		void getRequest(const String& host, int port, const String& path);

	private:
		httplib::Client*	m_client = nullptr;
	};
}
