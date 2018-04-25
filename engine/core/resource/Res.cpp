#include "Res.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	static map<String, Res*>::type				g_ress;

	Res::Res(const ResourcePath& path)
		: m_refCount(1)
	{
		m_path = path;

		auto it = g_ress.find(path.getPath());
		if (it == g_ress.end())
		{
			g_ress[path.getPath()] = this;
		}
		else
		{
			EchoLogError("create resource multi times");
		}
	}

	Res::~Res()
	{
		auto it = g_ress.find(m_path.getPath());
		if (it != g_ress.end())
		{
			g_ress.erase(it);
		}
		else
		{
			EchoLogError("can't delete resource for cache");
		}
	}

	// get res
	Res* Res::get(const ResourcePath& path)
	{
		auto it = g_ress.find(path.getPath());
		if (it != g_ress.end())
		{
			return it->second;
		}

		return nullptr;
	}

	// release
	void Res::release()
	{

	}
}