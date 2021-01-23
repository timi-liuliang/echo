#pragma once

#include "../pg_node.h"

namespace Echo
{
	class PGConnnect : public PGNode
	{
		ECHO_CLASS(PGConnnect, PGNode)

	public:
		PGConnnect();
		virtual ~PGConnnect();

		// calculate
		virtual void play(PCGData& data) override;

	protected:
	};
}
