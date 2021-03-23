#pragma once

#include "pcg_connect_point.h"

namespace Echo
{
	class PCGConnect
	{
	public:
		PCGConnect(PCGConnectPoint* from, PCGConnectPoint* to);
		virtual ~PCGConnect();

		// from
		PCGConnectPoint* getFrom() { return m_from; }

		// to
		PCGConnectPoint* getTo() { return m_to; }

	protected:
		PCGConnectPoint* m_from = nullptr;
		PCGConnectPoint* m_to = nullptr;
	};
}
