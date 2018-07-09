#include "object.h"
#include "engine/core/resource/Res.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include <thirdparty/pugixml/pugiconfig.hpp>

namespace Echo
{
	static map<i32, Object*>::type g_objs;

	Object::Object()
	{
		// begin with 1
		static i32 id = 1;
		m_id = id++;

		g_objs[m_id] = this;
	}

	Object::~Object()
	{
		auto it = g_objs.find(m_id);
		if (it != g_objs.end())
		{
			g_objs.erase(it);
		}
		else
		{
			EchoLogError("Object isn't exist. destruct failed.");
		}
	}

	// get by id
	Object* Object::getById(i32 id)
	{
		auto it = g_objs.find(id);
		if (it!=g_objs.end())
		{
			return it->second;
		}

		return nullptr;
	}

	// get class name
	const String& Object::getClassName() const
	{
		static String className = "Object";
		return className;
	}

	// propertys (script property or dynamic property)
	const PropertyInfos& Object::getPropertys() 
	{ 
		return m_propertys;
	}

	// clear propertys
	void Object::clearPropertys()
	{
		m_propertys.clear();
	}

	// register property
	bool Object::registerProperty(const String& className, const String& propertyName, const Variant::Type type)
	{
		PropertyInfoDynamic* info = EchoNew(PropertyInfoDynamic);
		info->m_name = propertyName;
		info->m_type = type;
		info->m_className = className;

		m_propertys.push_back(info);

		return true;
	}

	// instance res
	Object* Object::instanceObject(void* pugiNode)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		Echo::String name = xmlNode->attribute("name").value();
		Echo::String className = xmlNode->attribute("class").value();

		Object* res = Echo::Class::create<Object*>(className);
		if (res)
		{
			res->setName(name);

			loadPropertyRecursive(pugiNode, res, className);

			return res;
		}

		return  nullptr;
	}

	// remember property recursive
	void Object::loadPropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		// load parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if (parentClassName != "Object")
				loadPropertyRecursive(pugiNode, classPtr, parentClassName);
		}

		// load property
		loadPropertyValue(pugiNode, classPtr, className, PropertyInfo::Static);
		loadPropertyValue(pugiNode, classPtr, className, PropertyInfo::Dynamic);
	}

	// load propertys value
	void Object::loadPropertyValue(void* pugiNode, Echo::Object* classPtr, const Echo::String& className, i32 flag)
	{
		pugi::xml_node* xmlNode = (pugi::xml_node*)pugiNode;

		Echo::PropertyInfos propertys;
		Echo::Class::getPropertys(className, classPtr, propertys, flag);

		// iterator
		for (const Echo::PropertyInfo* prop : propertys)
		{
			if (prop->m_type == Variant::Type::Object)
			{
				for (pugi::xml_node propertyNode = xmlNode->child("property"); propertyNode; propertyNode = propertyNode.next_sibling("property"))
				{
					String propertyName = propertyNode.attribute("name").as_string();
					if (propertyName == prop->m_name)
					{
						String path = propertyNode.attribute("path").as_string();
						if (!path.empty())
						{
							Res* res = Res::get(path);
							Class::setPropertyValue(classPtr, prop->m_name, res);
						}
						else
						{
							pugi::xml_node objNode = propertyNode.child("obj");
							Object* obj = instanceObject(&objNode);
							Class::setPropertyValue(classPtr, prop->m_name, obj);
						}

						break;
					}
				}
			}
			else
			{
				Echo::Variant var;
				String valueStr = xmlNode->attribute(prop->m_name.c_str()).value();
				if (!valueStr.empty())
				{
					var.fromString(prop->m_type, valueStr);
					Class::setPropertyValue(classPtr, prop->m_name, var);
				}
				else
				{
					EchoLogInfo("Property [%s] not exist. when instance Object", prop->m_name.c_str());
				}
			}
		}
	}

	// remember property recursive
	void Object::savePropertyRecursive(void* pugiNode, Echo::Object* classPtr, const Echo::String& className)
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
		for (Echo::PropertyInfo* prop : propertys)
		{
			Echo::Variant var;
			Echo::Class::getPropertyValue(classPtr, prop->m_name, var);
			if (var.getType() == Variant::Type::Object)
			{
				Object* obj = var.toObj();
				if (obj)
				{
					pugi::xml_node propertyNode = xmlNode->append_child("property");
					propertyNode.append_attribute("name").set_value(prop->m_name.c_str());
					if (!obj->getPath().empty())
					{
						propertyNode.append_attribute("path").set_value(obj->getPath().c_str());
					}
					else
					{
						pugi::xml_node objNode = propertyNode.append_child("obj");
						savePropertyRecursive(&obj, obj, obj->getClassName());
					}
				}
			}
			else
			{
				Echo::String varStr = var.toString();
				xmlNode->append_attribute(prop->m_name.c_str()).set_value(varStr.c_str());
			}
		}
	}
}