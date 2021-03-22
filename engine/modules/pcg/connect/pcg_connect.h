#pragma once

#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	class PCGConnnect
	{
	public:
		PCGConnnect(std::shared_ptr<PCGConnectPoint> from, std::shared_ptr<PCGConnectPoint> to);
		virtual ~PCGConnnect();

	protected:
		std::shared_ptr<PCGConnectPoint>	m_from;
		std::shared_ptr<PCGConnectPoint>	m_to;
	};
}
