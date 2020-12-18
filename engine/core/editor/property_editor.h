#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	struct PropertyEditorFactory;
	class PropertyEditor
	{
	public:
		PropertyEditor();
		virtual ~PropertyEditor();

	public:
		// register|create editor
		static void registerEditor(const char* className, const char* propertyName, PropertyEditorFactory* factory);
		static PropertyEditor* createEditor(const char* className, const char* propertyName);

	protected:
	};

	struct PropertyEditorFactory
	{
		virtual PropertyEditor* create() = 0;
	};

	template<typename T>
	struct PropertyEditorFactoryT : public PropertyEditorFactory
	{
		PropertyEditorFactoryT(const char* className, const char* propertyName)
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


