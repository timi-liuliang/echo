#include "pcg_connect.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PCGConnect::PCGConnect(PCGConnectPoint* from, PCGConnectPoint* to)
		: m_from(from)
		, m_to(to)
	{
	}

	PCGConnect::~PCGConnect()
	{
	}
}
