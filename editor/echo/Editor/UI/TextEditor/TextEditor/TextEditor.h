#pragma once

#include <QMainWindow>
#include "ui_TextEditor.h"
#include "QProperty.hpp"
#include "LuaSyntaxHighLighter.h"
#include <engine/core/script/lua/lua_script.h>

namespace Studio
{
	class TextEditor : public QWidget, public Ui_TextEditorPanel
	{
		Q_OBJECT

	public:
		TextEditor(QWidget* parent = 0);
		~TextEditor();

		// open lua file
		void open( const Echo::String& fullPath);

		// get current edit lua file path
		const Echo::String& getFilePath();

	public slots :
		// save
		void save();

	protected slots:
		// on text changed
		void onTextChanged();

		// update title display
		void updateTitle();
        
    public: signals:
        // title changed
        void titleChanged(TextEditor* editor);

	private:
		Echo::String			m_pathName;
		Echo::String			m_content;
		SyntaxHighLighter*		m_syntaxHighLighter = nullptr;
	};
}
