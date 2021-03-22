#include "pcg_connect.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PCGConnnect::PCGConnnect(std::shared_ptr<PCGConnectPoint> from, std::shared_ptr<PCGConnectPoint> to)
		: m_from(from)
		, m_to(to)
	{

	}

	PCGConnnect::~PCGConnnect()
	{

	}
}
