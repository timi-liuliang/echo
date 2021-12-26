#include "net_module.h"
#include "http_server.h"
#include "http_client.h"
#include "editor/http_server_editor.h"
#include "editor/http_client_editor.h"

namespace Echo
{
	DECLARE_MODULE(NetModule)

	NetModule::NetModule()
	{

	}

	NetModule::~NetModule()
	{
	}

	NetModule* NetModule::instance()
	{
		static NetModule* inst = EchoNew(NetModule);
		return inst;
	}

	void NetModule::bindMethods()
	{

	}

	void NetModule::registerTypes()
	{
		Class::registerType<HttpServer>();
		Class::registerType<HttpClient>();

		CLASS_REGISTER_EDITOR(HttpClient, HttpClientEditor)
		CLASS_REGISTER_EDITOR(HttpServer, HttpServerEditor)
	}

	void NetModule::update(float elapsedTime)
	{
	}
}
