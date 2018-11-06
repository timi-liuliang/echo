#include "object_editor.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	static map<String, ObjectEditorFactory*>::type	g_objEditorFactorys;

	ObjectEditor::ObjectEditor(Object* object)
		: m_object(object)
	{

	}

	// register editor
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

	// create editor
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
#endif
}