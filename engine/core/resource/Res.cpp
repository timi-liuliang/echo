#include "Res.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

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

		const Echo::PropertyInfos& propertys = Echo::Class::getPropertys(className);
		for (const Echo::PropertyInfo& prop : propertys)
		{
			Echo::Variant var;
			Echo::Class::getPropertyValue(classPtr, prop.m_name, var);
			Echo::String varStr = var.toString();

			xmlNode->append_attribute(prop.m_name.c_str()).set_value(varStr.c_str());
		}
	}
}