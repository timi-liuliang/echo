#include "object_editor.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/main/Engine.h"
#include "engine/core/io/io.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	static map<String, ObjectEditorFactory*>::type	g_objEditorFactorys;

	ObjectEditor::ObjectEditor(Object* object)
		: m_object(object)
	{

	}

	void ObjectEditor::registerEditor(const char* objName, ObjectEditorFactory* factory)
	{
		if (g_objEditorFactorys.find(objName) == g_objEditorFactorys.end())
		{
			g_objEditorFactorys[objName] = factory;
		}
		else
		{
			EchoLogError("object editor factory for [%s] has existed", objName);
		}
	}

	ObjectEditor* ObjectEditor::createEditor(Object* object)
	{
		if (!Engine::instance()->getConfig().m_isGame)
		{
			auto it = g_objEditorFactorys.find(object->getClassName());
			if (it != g_objEditorFactorys.end())
			{
				return it->second->create(object);
			}
		}

		return nullptr;
	}

	bool ObjectEditor::isThumbnailStatic(Object* object)
	{
		if(object->getEditor())
			return object->getEditor()->isThumbnailStatic();

		return true;
	}

	ImagePtr ObjectEditor::getThumbnail(Object* object)
	{
		if (object)
		{
			if (object->getEditor())
			{
				ImagePtr icon = object->getEditor()->getThumbnail();
				if(icon)
					return icon;
			}

			String className = object->getClassName();
			ClassInfo* info = Echo::Class::getClassInfo(className);
			String moduleName = (info && !info->m_module.empty()) ? info->m_module : "";
			if (!moduleName.empty())
			{
				String iconPath = StringUtil::Format("Module://%s/editor/icon/%s.png", moduleName.c_str(), className.c_str());
				if (IO::instance()->isExist(iconPath))
				{
					ImagePtr icon = Image::loadFromFile(iconPath);
					if (icon)
						return icon;
				}
			}
		}

		return nullptr;
	}
#endif
}