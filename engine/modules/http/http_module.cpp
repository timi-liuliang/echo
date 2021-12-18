#include "http_module.h"
#include "http_server.h"
#include "http_client.h"
#include "editor/http_server_editor.h"
#include "editor/http_client_editor.h"

namespace Echo
{
	DECLARE_MODULE(HttpModule)

	HttpModule::HttpModule()
	{

	}

	HttpModule::~HttpModule()
	{
	}

	HttpModule* HttpModule::instance()
	{
		static HttpModule* inst = EchoNew(HttpModule);
		return inst;
	}

	void HttpModule::bindMethods()
	{

	}

	void HttpModule::registerTypes()
	{
		Class::registerType<HttpServer>();
		Class::registerType<HttpClient>();

		CLASS_REGISTER_EDITOR(HttpClient, HttpClientEditor)
		CLASS_REGISTER_EDITOR(HttpServer, HttpServerEditor)
	}

	void HttpModule::update(float elapsedTime)
	{
	}
}
