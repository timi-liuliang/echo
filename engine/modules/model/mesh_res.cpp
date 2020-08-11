#include "mesh_res.h"
#include "engine/core/io/MemoryReader.h"

namespace Echo
{
	MeshRes::MeshRes(const ResourcePath& path)
		: Res(path)
	{
		load(path);
	}

	MeshRes::~MeshRes()
	{

	}

	void MeshRes::bindMethods()
	{

	}

	Res* MeshRes::load(const ResourcePath& path)
	{
		if (!path.isEmpty())
		{
			MeshRes* res = EchoNew(MeshRes);
			return res;
		}

		return nullptr;
	}

	void MeshRes::save()
	{

	}
}
