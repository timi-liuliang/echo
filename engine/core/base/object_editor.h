#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class Object;
	struct ObjectEditorFactory;
	class ObjectEditor
	{
	public:
		ObjectEditor(Object* object);

		// register editor
		static void registerEditor(const char* objName, ObjectEditorFactory* factory);

		// create editor
		static ObjectEditor* createEditor(Object* object);

		/* editor update self*/
		virtual void editor_update_self() {}

		/* get node icon*/
		virtual const char* getEditorIcon() const { return ""; }

		/* on editor select this node*/
		virtual void onEditorSelectThisNode() {}

		// object
		Object* getObject() { return m_object; }

	protected:
		Object*			m_object;
	};

	struct ObjectEditorFactory
	{
		virtual ObjectEditor* create(Object* object)=0;
	};

	template<typename T>
	struct ObjectEditorFactoryT : public ObjectEditorFactory
	{
		ObjectEditorFactoryT(const char* objName)
		{
			ObjectEditor::registerEditor(objName, this);
		}

		virtual ObjectEditor* create(Object* object) override
		{
			return EchoNew(T(object));
		}
	};
}

#define REGISTER_OBJECT_EDITOR(OBJECT, EDITOR) static Echo::ObjectEditorFactoryT<EDITOR> G_OBJECT_EDIT_##EDITOR##_REGISTER(#OBJECT);

// define node edit interface for echo editor
#define ECHO_EDITOR_INTERFACE									\
public:															\
	/* on editor select this node*/								\
	Echo::ObjectEditor* getEditor() { return m_objectEditor; }	\
																\
protected:														\
	Echo::ObjectEditor*			m_objectEditor;					\


#else
#define ECHO_EDITOR_INTERFACE
#endif