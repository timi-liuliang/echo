#include "property_editor.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	static map<String, PropertyEditorFactory*>::type	g_propertyEditorFactorys;

	PropertyEditor::PropertyEditor()
	{

	}

	PropertyEditor::~PropertyEditor()
	{

	}

	void PropertyEditor::registerEditor(const String& className, const String& propertyName, PropertyEditorFactory* factory)
	{
		String finalName = StringUtil::Format("%s.%s", className.c_str(), propertyName.c_str());
		if (g_propertyEditorFactorys.find(finalName) == g_propertyEditorFactorys.end())
		{
			g_propertyEditorFactorys[finalName] = factory;
		}
		else
		{
			EchoLogError("property editor factory for [%s] has existed", finalName);
		}
	}

	PropertyEditor* PropertyEditor::createEditor(const String& className, const String& propertyName)
	{
		PropertyEditorFactory* factory = getFactory(className, propertyName);
		if (factory)
		{
			PropertyEditor* property = factory->create();
			property->m_className = className;
			property->m_propertyName = propertyName;

			return property;
		}

		return nullptr;
	}

	PropertyEditorFactory* PropertyEditor::getFactory(const String& className, const String& propertyName)
	{
		if (!Engine::instance()->getConfig().m_isGame)
		{
			String finalName = StringUtil::Format("%s.%s", className.c_str(), propertyName.c_str());
			auto it = g_propertyEditorFactorys.find(finalName);
			if (it != g_propertyEditorFactorys.end())
			{
				return it->second;
			}
		}

		return nullptr;
	}

#endif
}