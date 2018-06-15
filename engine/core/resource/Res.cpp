#include "Res.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/io/IO.h"
#include "engine/core/util/PathUtil.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	static map<String, Res*>::type	g_ress;
	static map<String, Res::ResFun>::type g_resFuncs;

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
	}

	// resister res
	void Res::registerRes(const String& ext, RES_CREATE_FUNC cfun, RES_LOAD_FUNC lfun)
	{
		ResFun fun;
		fun.m_cfun = cfun;
		fun.m_lfun = lfun;

		g_resFuncs[ext] = fun;
	}

	// get res
	Res* Res::get(const ResourcePath& path)
	{
		auto it = g_ress.find(path.getPath());
		if (it != g_ress.end())
		{
			return it->second;
		}

		// get load fun
		String ext = PathUtil::GetFileExt(path.getPath(), true);
		map<String, Res::ResFun>::type::iterator itfun = g_resFuncs.find(ext);
		if (itfun != g_resFuncs.end())
		{
			Res* res = itfun->second.m_lfun(path);
			if (!res)
			{
				EchoLogError("Res::get file [%s] failed.", path.getPath().c_str());
			}

			return res;
		}

		return nullptr;
	}

	// create by extension
	ResPtr Res::create(const String& extension)
	{
		String ext = extension;
		map<String, Res::ResFun>::type::iterator itfun = g_resFuncs.find(ext);
		if (itfun != g_resFuncs.end())
		{
			Res* res = itfun->second.m_cfun();
			if (res)
				return res;	
		}

		EchoLogError("Res::create failed. Unknown extension [%s]", extension.c_str());
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

	// load
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
					Res* resNode = ECHO_DOWN_CAST<Res*>(instanceObject(&root));
					resNode->setPath(path);

					g_ress[path.getPath()] = resNode;

					return resNode;
				}
			}
		}

		return nullptr;
	}

	// save
	void Res::save()
	{
		String fullPath = IO::instance()->getFullPath(m_path.getPath());

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

	// remember property recursive
	void Res::savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		// save parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if (parentClassName != "Object")
				savePropertyRecursive(pugiNode, classPtr, parentClassName);
		}

		Echo::PropertyInfos propertys;
		Echo::Class::getPropertys(className, classPtr, propertys);
		for (const Echo::PropertyInfo* prop : propertys)
		{
			Echo::Variant var;
			Echo::Class::getPropertyValue(classPtr, prop->m_name, var);
			Echo::String varStr = var.toString();

			xmlNode->append_attribute(prop->m_name.c_str()).set_value(varStr.c_str());
		}
	}
}