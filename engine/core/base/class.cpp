#include "class.h"
#include "object.h"
#include <map>
#include "engine/core/log/Log.h"
#include "engine/core/main/module.h"

namespace Echo
{
	static std::map<String, ObjectFactory*>*	g_classInfos = nullptr;

	void ObjectFactory::registerProperty(PropertyInfo* property)
	{
		PropertyInfo* pi = getProperty(property->m_name);
		if (!pi)
		{
			m_classInfo.m_propertyInfos.push_back(property);
		}
		else
		{
			EchoLogError("property [%s] already exist", property->m_name.c_str());
		}
	}
    
    const char* ObjectFactory::getCurrentRegisterModuleName()
    {
        return Module::getCurrentRegisterModuleName();
    }
    
#ifdef ECHO_EDITOR_MODE
    void ObjectFactory::initEditor(Object* obj)
    {
        obj->initEditor();
    }
#endif

	// add class
	void Class::addClass(const String& className, ObjectFactory* objFactory)
	{
		if (!g_classInfos)
		{
			g_classInfos = new std::map<String, ObjectFactory*>();
		}

		(*g_classInfos)[className] = objFactory;
	}

	// get class info
	ClassInfo* Class::getClassInfo(const String& className)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			return &(it->second->m_classInfo);
		}

		return nullptr;
	}

	// is derived from
	bool Class::isDerivedFrom(const String& className, const String& parentClassName)
	{
		String parent;
		if (getParentClass(parent, className))
		{
			if (parent == parentClassName)
				return true;
			else
				return isDerivedFrom(parent, parentClassName);
		}

		return false;
	}

	// is virtual
	bool Class::isVirtual(const String& className)
	{
		ClassInfo* cinfo = getClassInfo(className);	
		return cinfo ? cinfo->m_virtual : true;
	}

	// is singleton
	bool Class::isSingleton(const String& className)
	{
		ClassInfo* cinfo = getClassInfo(className);
		return cinfo ? cinfo->m_singleton : true;
	}

	// get parent class name
	bool Class::getParentClass(String& parentClassName, const String& className)
	{
		ClassInfo* cinfo = getClassInfo(className);
		if (cinfo)
		{
			parentClassName = cinfo->m_parent;
			return true;
		}

		return false;
	}

	// get all child class
	bool Class::getChildClasses(StringArray& childClasses, const String& className, bool recursive)
	{
		if (!g_classInfos)
			return false;

		for (auto it : *g_classInfos)
		{
			if (it.second->m_classInfo.m_parent == className)
			{
				childClasses.push_back(it.first);
				if (recursive)
				{
					getChildClasses(childClasses, it.first, recursive);
				}
			}
		}

		return !childClasses.empty();
	}

	// get all class names
	size_t Class::getAllClasses(StringArray& classes)
	{
		for (auto& it : *g_classInfos)
		{
			classes.push_back(it.first);
		}

		return classes.size();
	}

	Object* Class::create(const String& className)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			Object* obj = it->second->create();
			if (obj)
				obj->setName(className);

			return obj;
		}

		return nullptr;
	}

	// register method
	bool Class::registerMethodBind(const String& className, const String& methodName, ClassMethodBind* method)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			// c
			it->second->registerMethod(methodName, method);

			// lua
			LuaBinder::instance()->registerClassMethod(className, methodName, method);
            
            return true;
		}

		return false;
	}
    
    // register signal
    bool Class::registerSignal(const String& className, const String& signalName, ClassMethodBind* getSignalMethod)
    {
        auto it = g_classInfos->find(className);
        if (it != g_classInfos->end())
        {
            // c
            it->second->registerSignalGetMethod(signalName, getSignalMethod);
            
            return true;
        }
        
        return false;
    }

	// get method
	ClassMethodBind* Class::getMethodBind(const String& className, const String& methodName)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			return it->second->getMethodBind(methodName);
		}

		return nullptr;
	}
    
    // get signal by class name
    Signal* Class::getSignal(const String& className, Object* classPtr, const String& signalName)
    {
        auto it = g_classInfos->find(className);
        if (it != g_classInfos->end())
        {
            ClassMethodBind* method = it->second->getSignalGetMethod(signalName);
            if(method)
            {
                Variant::CallError error;
                return method->call(classPtr, nullptr, 0, error);
            }
        }
        
        return nullptr;
    }

    // get signal
    Signal* Class::getSignal(Object* classPtr, const String& signalName)
    {
        String className = classPtr->getClassName();
        do
        {
            Signal* signal = getSignal(className, classPtr, signalName);
            if (signal)
            {
                return signal;
            }
            
        } while (getParentClass(className, className));
        
        return nullptr;
    }

	// add property
	bool Class::registerProperty(const String& className, const String& propertyName, const Variant::Type type, PropertyHint hint, const String& hintStr, const String& getter, const String& setter)
	{
		auto it = g_classInfos->find(className);
		if (it != g_classInfos->end())
		{
			PropertyInfoStatic* info = EchoNew(PropertyInfoStatic);
			info->m_name = propertyName;
			info->m_type = type;
			info->m_hint = hint;
			info->m_hintStr = hintStr;
			info->m_setter = setter;
			info->m_getter = getter;
			info->m_setterMethod = getMethodBind(className, setter);
			info->m_getterMethod = getMethodBind(className, getter);

			it->second->registerProperty(info);
		}

		return true;
	}

	// get propertys
	ui32 Class::getPropertys(const String& className, Object* classPtr, PropertyInfos& propertys, i32 flag)
	{
		// static
		if (flag & PropertyInfo::Static)
		{
			auto it = g_classInfos->find(className);
			if (it != g_classInfos->end())
			{
				for (PropertyInfo* info : it->second->getPropertys())
				{
					propertys.push_back(info);
				}
			}
		}

		// dynamic
		if (flag & PropertyInfo::Dynamic)
		{
			const PropertyInfos& dynamicPropertys = classPtr->getPropertys();
			for (PropertyInfo* pi : dynamicPropertys)
			{
				if (((PropertyInfoDynamic*)pi)->m_className == className)
				{
					propertys.push_back(pi);
				}
			}
		}

		return static_cast<ui32>(propertys.size());
	}
    
    PropertyInfo* Class::getProperty(Object* classPtr, const String& propertyName)
    {
        String className = classPtr->getClassName();
        do
        {
            PropertyInfo* pi = getProperty(className, classPtr, propertyName);
            if (pi)
            {
                return pi;
            }
            
        } while (getParentClass(className, className));
        
        return nullptr;
    }

	// get property
	PropertyInfo* Class::getProperty(const String& className, Object* classPtr, const String& propertyName)
	{
		PropertyInfos propertys;
		getPropertys(className, classPtr, propertys);
		for (PropertyInfo* p : propertys)
		{
			if (p->m_name == propertyName)
			{
				return p;
			}
		}

		return nullptr;
	}

	// get property value
	bool Class::getPropertyValue(Object* classPtr, const String& propertyName, Variant& oVar)
	{
		String className = classPtr->getClassName();
		do
		{
			PropertyInfo* pi = getProperty(className, classPtr, propertyName);
			if (pi)
			{
				if (pi->getPropertyValue(classPtr, propertyName, oVar))
					return true;
			}

		} while (getParentClass(className, className));

		return false;
	}

	// get property type
	Variant::Type Class::getPropertyType(Object* classPtr, const String& propertyName)
	{
		String className = classPtr->getClassName();
		do
		{
			PropertyInfo* pi = getProperty(className, classPtr, propertyName);
			if (pi)
				return pi->m_type;

		} while (getParentClass(className, className));

		return Variant::Type::Unknown;
	}

	// set property value
	// set property value
	bool Class::setPropertyValue(Object* classPtr, const String& propertyName, const Variant& propertyValue)
	{
		String className = classPtr->getClassName();
		do
		{
			PropertyInfo* pi = getProperty(className, classPtr, propertyName);
			if (pi)
			{
				pi->setPropertyValue(classPtr, propertyName, propertyValue);

				return true;
			}

		} while (getParentClass(className, className));

		return false;
	}
}
