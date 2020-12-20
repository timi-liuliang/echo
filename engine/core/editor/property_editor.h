#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/memory/MemAllocDef.h"
#include "qt/QWidget.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	struct PropertyEditorFactory;
	class PropertyEditor : public QWidget
	{
	public:
		PropertyEditor();
		virtual ~PropertyEditor();

		// set
		virtual void setObject(Object* object) { m_object = object; }

	public:
		// register|create editor
		static void registerEditor(const String& className, const String& propertyName, PropertyEditorFactory* factory);
		static PropertyEditor* createEditor(const String& className, const String& propertyName);

		// get factory
		static PropertyEditorFactory* getFactory(const String& className, const String& propertyName);

	protected:
		Object*		m_object = nullptr;
		String		m_className;
		String		m_propertyName;
	};

	struct PropertyEditorFactory
	{
		virtual PropertyEditor* create() = 0;
	};

	template<typename T>
	struct PropertyEditorFactoryT : public PropertyEditorFactory
	{
		PropertyEditorFactoryT(const String& className, const String& propertyName)
		{
			PropertyEditor::registerEditor(className, propertyName, this);
		}

		virtual PropertyEditor* create() override
		{
			return EchoNew(T);
		}
	};
}

#define REGISTER_PROPERTY_EDITOR(OBJECT, PROPERTY, EDITOR) static Echo::PropertyEditorFactoryT<EDITOR> G_PROPERTY_EDIT_##EDITOR##_REGISTER(#OBJECT, PROPERTY);
#else
#define REGISTER_PROPERTY_EDITOR(OBJECT, PROPERTY, EDITOR)
#endif


