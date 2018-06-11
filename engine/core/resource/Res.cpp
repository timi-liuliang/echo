#include "Res.h"
#include "engine/core/log/LogManager.h"

namespace Echo
{
	static map<String, Res*>::type	g_ress;

	Res::Res()
		: m_refCount(0)
		, m_isLoaded(false)
	{

	}

	Res::Res(const ResourcePath& path)
		: m_refCount(0)
		, m_isLoaded(false)
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

	// bind methods to script
	void Res::bindMethods()
	{
		CLASS_BIND_METHOD(Res, getPath, DEF_METHOD("getPath"));
		CLASS_BIND_METHOD(Res, setPath, DEF_METHOD("setPath"));

		CLASS_REGISTER_PROPERTY(Res, "Path", Variant::Type::ResourcePath, "getPath", "setPath");
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
	void Res::subRefCount()
	{
		m_refCount--;
		if (m_refCount <= 0)
		{
			ECHO_DELETE_T(this, Res);
		}
	}
}