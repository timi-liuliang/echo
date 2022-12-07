#pragma once

#include "engine/core/scene/node.h"
#include <cpp-httplib/httplib.h>

namespace Echo
{
	class HttpServer : public Node 
	{
		ECHO_CLASS(HttpServer, Node)

	public:
		HttpServer();
		virtual ~HttpServer();

		// Ip
		const String& getIp() const { return m_ip; }
		void setIp(const String& ip) { m_ip = ip; }

		// Port
		i32 getPort() const { return m_port; }
		void setPort(i32 port) { m_port = port; }

		// Listen
		void listen();

	private:
		String			m_ip = "127.0.0.1";
		i32				m_port = 8090;
		httplib::Server m_server;
	};
}
