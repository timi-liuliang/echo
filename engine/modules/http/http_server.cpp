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
		CLASS_BIND_METHOD(HttpServer, getIp, DEF_METHOD("getIp"));
		CLASS_BIND_METHOD(HttpServer, setIp, DEF_METHOD("setIp"));
		CLASS_BIND_METHOD(HttpServer, getPort, DEF_METHOD("getPort"));
		CLASS_BIND_METHOD(HttpServer, setPort, DEF_METHOD("setPort"));
		CLASS_BIND_METHOD(HttpServer, listen, DEF_METHOD("listen"));

		CLASS_REGISTER_PROPERTY(HttpServer, "Ip", Variant::Type::String, "getIp", "setIp");
		CLASS_REGISTER_PROPERTY(HttpServer, "Port", Variant::Type::Int, "getPort", "setPort");
	}

	void HttpServer::listen()
	{
		m_server.listen(m_ip.c_str(), m_port);
	}
}
