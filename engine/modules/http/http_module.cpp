#include "http_module.h"
#include "http_server.h"
#include "http_client.h"

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
	}

	void HttpModule::update(float elapsedTime)
	{
	}
}
