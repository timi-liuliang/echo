#include "http_client.h"

namespace Echo
{
	HttpClient::HttpClient()
	{
	}

	HttpClient::~HttpClient()
	{
		EchoSafeDelete(m_client, Client);
	}

	void HttpClient::bindMethods()
	{
		CLASS_BIND_METHOD(HttpClient, getRequest);
		CLASS_BIND_METHOD(HttpClient, download);
	}

	bool HttpClient::init(const String& host, int port)
	{
		if (!m_client || m_host != host || m_port != port)
		{
			EchoSafeDelete(m_client, Client);

			m_host = host;
			m_port = port;
			m_client = EchoNew(httplib::Client(host.c_str(), port));
		}

		return m_client ? true : false;
	}

	void HttpClient::getRequest(const String& host, int port, const String& path)
	{
		if (init(host, port)) 
		{
			httplib::Result result = m_client->Get(path.c_str());
		}	
	}

	void HttpClient::download(const String& host, int port, const String& path, const String& savePath)
	{
		if (init(host, port))
		{
			httplib::Result result = m_client->Get(path.c_str());
		}
	}
}
