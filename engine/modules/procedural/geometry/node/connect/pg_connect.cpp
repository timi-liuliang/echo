#include "pg_connect.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PGConnnect::PGConnnect()
	{

	}

	PGConnnect::~PGConnnect()
	{

	}

	void PGConnnect::bindMethods()
	{
	}

	void PGConnnect::play(PCGData& data)
	{
		m_dirtyFlag = false;
	}
}
