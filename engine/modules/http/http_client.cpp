#include "http_client.h"

namespace Echo
{
	HttpClient::HttpClient()
	{
	}

	HttpClient::~HttpClient()
	{
	}

	void HttpClient::bindMethods()
	{
		CLASS_BIND_METHOD(HttpClient, getRequest);
	}

	void HttpClient::getRequest(const String& host, int port, const String& path)
	{
		m_client = EchoNew(httplib::Client(host.c_str(), port));
		httplib::Result result = m_client->Get(path.c_str());
	}
}
