#pragma once

#include "../../../procedural_geometry.h"

namespace Echo
{
	class PGGrid : public PGNode
	{
		ECHO_CLASS(PGGrid, PGNode)

	public:
		PGGrid();
		virtual ~PGGrid();

		// calculate
		virtual void play(PCGData& data) override;
	};
}
