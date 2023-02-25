#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/render/base/image/image.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class Object;
	struct ObjectEditorFactory;
	class ObjectEditor
	{
	public:
		ObjectEditor(Object* object);
        virtual ~ObjectEditor() {}

		// register editor
		static void registerEditor(const char* objName, ObjectEditorFactory* factory);

		// create editor
		static ObjectEditor* createEditor(Object* object);

		// Is thumbnail static
		static bool isThumbnailStatic(Object* object);

		// Get thumbnail
		static ImagePtr getThumbnail(Object* object);

	public:
		/* editor update self*/
		virtual void editor_update_self() {}

		// is thumbnail static (every instance has same thumbnail)
		virtual bool isThumbnailStatic() const { return true; }

		// get thumbnail
		virtual ImagePtr getThumbnail() const { return nullptr; }

		// is selected
		bool isSelected() const { return m_isSelected; }

		/* on editor selected this node*/
		virtual void onEditorSelectThisNode() { m_isSelected = true; }

		// on editor unselected this node
		virtual void onEditorUnSelectThisNode() { m_isSelected = false; }

		// object
		Object* getObject() { return m_object; }

	public:
		// post process
		virtual void postEditorCreateObject() {}

	protected:
		Object*			m_object;
		bool			m_isSelected = false;
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

#define CLASS_REGISTER_EDITOR(OBJECT, EDITOR) static Echo::ObjectEditorFactoryT<EDITOR> G_OBJECT_EDIT_##EDITOR##_REGISTER(#OBJECT);

// define node edit interface for echo editor
#define ECHO_EDITOR_INTERFACE									\
public:															\
	/* initialize editor */										\
	void initEditor()											\
	{															\
		m_objectEditor = Echo::ObjectEditor::createEditor(this);\
	}															\
																\
	/* on editor select this node*/								\
	Echo::ObjectEditor* getEditor()                             \
    {                                                           \
        if(!m_objectEditor)                                     \
            initEditor();                                       \
                                                                \
        return m_objectEditor;                                  \
    }	                                                        \
																\
protected:														\
	Echo::ObjectEditor*			m_objectEditor;					\


#else
#define ECHO_EDITOR_INTERFACE
#define CLASS_REGISTER_EDITOR(OBJECT, EDITOR)
#endif
