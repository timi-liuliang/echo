#include "Res.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include "engine/core/util/PathUtil.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	static std::unordered_map<String, Res*>			g_ress;
	static std::unordered_map<String, Res::ResFun>	g_resFuncs;

	static void addResToCache(const String& path, Res* res)
	{
		auto it = g_ress.find(path);
		if (it == g_ress.end())
		{
			g_ress[path] = res;
		}
		else
		{
			EchoLogError("create resource multi times");
		}
	}

	static void removeResFromCache(const String& path)
	{
		auto it = g_ress.find(path);
		if (it != g_ress.end())
		{
			g_ress.erase(it);
		}
		else
		{
			EchoLogError("can't delete resource for cache");
		}
	}

	Res::Res()
		: m_refCount(1)
		, m_isLoaded(false)
	{
		//EchoAssert(false);
	}

	Res::Res(const ResourcePath& path)
		: m_refCount(1)
		, m_isLoaded(false)
	{
		setPath(path.getPath());
	}

	Res::~Res()
	{
		removeResFromCache(m_path.getPath());
	}

	void Res::bindMethods()
	{
		CLASS_BIND_METHOD(Res, getPath, DEF_METHOD("getPath"));
		CLASS_BIND_METHOD(Res, setPath, DEF_METHOD("setPath"));

		CLASS_REGISTER_PROPERTY(Res, "Path", Variant::Type::String, "getPath", "setPath");
	}

	void Res::setPath(const String& path)
	{
		// remove res cache
		if (!m_path.getPath().empty())
			removeResFromCache(m_path.getPath());

		// add to res cache
		if (!path.empty() && m_path.setPath(path))
			addResToCache(m_path.getPath(), this);
		else
			EchoLogError("setPath [%s] failed", path.c_str());
	}

	void Res::registerRes(const String& className, const String& exts, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun)
	{
		ResFun fun;
		fun.m_class = className;
		fun.m_cfun = cfun;
		fun.m_lfun = lfun;

		StringArray extArray = StringUtil::Split(exts, "|");
		for (String& ext : extArray)
		{
			StringUtil::LowerCase(ext);
			fun.m_ext = ext;
			g_resFuncs[ext] = fun;
		}
	}

	Res* Res::get(const ResourcePath& path)
	{
		auto it = g_ress.find(path.getPath());
		if (it != g_ress.end())
		{
			return it->second;
		}

		// get load fun
		String ext = PathUtil::GetFileExt(path.getPath(), true);
		if (!ext.empty())
		{
			StringUtil::LowerCase(ext);
			std::unordered_map<String, Res::ResFun>::iterator itfun = g_resFuncs.find(ext);
			if (itfun != g_resFuncs.end())
			{
				Res* res = itfun->second.m_lfun(path);
				if (!res)
				{
					EchoLogError("Res::get file [%s] failed.", path.getPath().c_str());
				}

				return res;
			}

			EchoLogError("Res::get file [%s] failed. can't find load method for this type of resource", path.getPath().c_str());
		}

		return nullptr;
	}

	ResPtr Res::createByFileExtension(const String& extWithDot)
	{
		String ext = extWithDot;
		std::unordered_map<String, Res::ResFun>::iterator itfun = g_resFuncs.find(ext);
		if (itfun != g_resFuncs.end() && itfun->second.m_cfun)
		{
			Res* res = itfun->second.m_cfun();
			if (res)
				return res;	
		}

		EchoLogError("Res::create failed. Unknown extension [%s]", extWithDot.c_str());
		return nullptr;
	}

	const Res::ResFun* Res::getResFunByExtension(const String& extWithDot)
	{
		// get load fun
		String ext = extWithDot;
		StringUtil::LowerCase(ext);
		std::unordered_map<String, Res::ResFun>::iterator it = g_resFuncs.find(ext);
		if (it != g_resFuncs.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const Res::ResFun* Res::getResFunByClassName(const String& className)
	{
		for (auto& it : g_resFuncs)
		{
			if (it.second.m_class == className)
			{
				return &it.second;
			}
		}

		return nullptr;
	}

	void Res::subRefCount()
	{
		m_refCount--;
		if (m_refCount <= 0)
		{
			ECHO_DELETE_T(this, Res);
		}
	}

	Res* Res::load(const ResourcePath& path)
	{
		MemoryReader reader(path.getPath());
		if (reader.getSize())
		{
			pugi::xml_document doc;
			if (doc.load_buffer(reader.getData<char*>(), reader.getSize()))
			{
				pugi::xml_node root = doc.child("res");
				if (root)
				{
					Res* res = ECHO_DOWN_CAST<Res*>(instanceObject(&root));
					res->setPath(path.getPath());

					g_ress[path.getPath()] = res;

					return res;
				}
			}
		}

		return nullptr;
	}

	void Res::save()
	{
		String fullPath = IO::instance()->convertResPathToFullPath(m_path.getPath());

		pugi::xml_document doc;

		// declaration
		pugi::xml_node dec = doc.prepend_child(pugi::node_declaration);
		dec.append_attribute("version") = "1.0";
		dec.append_attribute("encoding") = "utf-8";

		// root node
		pugi::xml_node root = doc.append_child("res");

		root.append_attribute("name").set_value(getName().c_str());
		root.append_attribute("class").set_value(getClassName().c_str());
		savePropertyRecursive(&root, this, this->getClassName());

		doc.save_file(fullPath.c_str(), "\t", 1U, pugi::encoding_utf8);
	}
}
