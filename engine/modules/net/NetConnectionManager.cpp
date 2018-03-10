#include "NetConnectionManager.h"

#ifdef ECHO_USE_LIBEVENT
#include "Object/Root.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/http_struct.h>

#ifdef ECHO_PLATFORM_WINDOWS
	#include <WinSock2.h>
#endif
namespace Echo
{
	static void onRequest(evhttp_request* request, void* arg)
	{
		NetConnection* pThis = static_cast<NetConnection*>(arg); 
		pThis->onRequestComplete(request); 
	}

	NetConnection::NetConnection()
		: m_onRequestCompleteCallback(nullptr)
		, m_uri(nullptr)
		, m_con(nullptr)
	{

	}

	NetConnection::~NetConnection()
	{

	}

	void NetConnection::createConnection(const Echo::String& url)
	{

	}

	void NetConnection::setRequestCompleteCb(onRequestCompleteCb cb)
	{
		m_onRequestCompleteCallback = cb; 
	}

	void NetConnection::requestFile(const Echo::String& url, void* arg, const String& fileName)
	{
		auto* base = EchoNetConnectionManager->getNetConnectionBase();

		m_uri = evhttp_uri_parse(url.c_str());

		auto port = evhttp_uri_get_port(m_uri);
		auto host = evhttp_uri_get_host(m_uri);
		auto query = evhttp_uri_get_query(m_uri);
		auto path = evhttp_uri_get_path(m_uri);

		m_con = evhttp_connection_base_new(base, NULL, host, port == -1 ? 80 : port);
		auto* request = evhttp_request_new(onRequest, this);

		size_t len = (query ? strlen(query) : 0) + (path ? strlen(path) : 0) + 1;
		char *path_query = NULL;
		if (len > 1) {
			path_query = (char*)calloc(len, sizeof(char));
			sprintf(path_query, "%s?%s", path, query);
		}

		NetContext context = { fileName, arg };
		m_contexts[path_query] = context;

		evhttp_make_request(m_con, request, EVHTTP_REQ_GET, path_query ? path_query : "/");
		evhttp_add_header(request->output_headers, "Host", host);

		event_base_dispatch(base); 
	}

	void NetConnection::onRequestComplete(evhttp_request* request)
	{
		if (!request)
		{
			EchoLogError("Connection Failed!"); 
			return; 
		}

		switch (request->response_code)
		{
		case HTTP_OK:
		{	
			auto uri = evhttp_request_get_uri(request);
			auto buf = evhttp_request_get_input_buffer(request);

			auto iter = m_contexts.find(uri);
			if (iter == m_contexts.end())
			{
				EchoLogError("Net Context Lost! uri == %s", uri);
				return;
			}

			const auto& context = iter->second;
			size_t len = evbuffer_get_length(buf);
			unsigned char *data = (unsigned char*)malloc(len + 1);
			memset(data, 0, len); 

			unsigned char *outp = evbuffer_pullup(buf, len); 
			memcpy(data, outp, len); 

			if (m_onRequestCompleteCallback)
			{
				m_onRequestCompleteCallback(context.pThis, context.name, data, len, NetErrCode::HA_REQUEST_OK);
			}
			else
			{
				EchoLogDebug("[NetConnection::onRequestComplete:%d]:: NetConnection No Callback!", __LINE__);
			}

			free(data); 
			event_base_loopexit(EchoNetConnectionManager->getNetConnectionBase(), 0);
			break;
		}
		default:
			event_base_loopexit(EchoNetConnectionManager->getNetConnectionBase(), 0);
			return;
		}
	}

	NetConnectionManager::NetConnectionManager()
	{
#ifndef ECHO_PLATFORM_HTML5
		m_eventBase = event_base_new();
#endif

#ifdef ECHO_PLATFORM_WINDOWS
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		WSAStartup(wVersionRequested, &wsaData);
#endif
	}

	NetConnectionManager::~NetConnectionManager()
	{
#ifdef ECHO_PLATFORM_WINDOWS
		WSACleanup();
#endif
	}

	void NetConnectionManager::createNetConnection(const Echo::String& url)
	{
		auto iter = m_netConnectinos.find(url); 
		if (iter != m_netConnectinos.end())
		{
			EchoLogDebug("net connection already exist"); 
			return; 
		}

		NetConnection* conn = EchoNew(NetConnection); 
		m_netConnectinos[url] = conn; 
	}

	void NetConnectionManager::removeNetConnection(const Echo::String& url)
	{
		auto iter = m_netConnectinos.find(url);
		if (iter != m_netConnectinos.end())
		{
			m_netConnectinos.erase(iter); 
		}
	}

	NetConnection* NetConnectionManager::getNetConnection(const Echo::String& url)
	{
		auto iter = m_netConnectinos.find(url); 
		if (iter != m_netConnectinos.end())
		{
			return iter->second; 
		}

		return nullptr; 
	}
}

#endif