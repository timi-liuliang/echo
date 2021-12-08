#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class HttpClient : public Node
	{
		ECHO_CLASS(HttpClient, Node)

	public:
		HttpClient();
		virtual ~HttpClient();

	private:
	};
}
