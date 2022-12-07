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

		// Init
		bool init(const String& host, int port);

		// Get
		void getRequest(const String& host, int port, const String& path);

		// Download
		void download(const String& host, int port, const String& path, const String& savePath);

	private:
		String				m_host;
		i32					m_port = 0;
		httplib::Client*	m_client = nullptr;
	};
}
