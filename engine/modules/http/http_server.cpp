#include "http_server.h"

namespace Echo
{
	HttpServer::HttpServer()
	{
	}

	HttpServer::~HttpServer()
	{
	}

	void HttpServer::bindMethods()
	{
		CLASS_BIND_METHOD(HttpServer, getIp);
		CLASS_BIND_METHOD(HttpServer, setIp);
		CLASS_BIND_METHOD(HttpServer, getPort);
		CLASS_BIND_METHOD(HttpServer, setPort);
		CLASS_BIND_METHOD(HttpServer, listen);

		CLASS_REGISTER_PROPERTY(HttpServer, "Ip", Variant::Type::String, getIp, setIp);
		CLASS_REGISTER_PROPERTY(HttpServer, "Port", Variant::Type::Int, getPort, setPort);
	}

	void HttpServer::listen()
	{
		m_server.listen(m_ip.c_str(), m_port);
	}
}
