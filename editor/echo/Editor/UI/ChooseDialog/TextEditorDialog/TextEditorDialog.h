#pragma once

#include <QDialog>
#include "LuaSyntaxHighLighter.h"
#include "ui_TextEditorDialog.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/base/object.h>

namespace Studio
{
	class TextEditorDialog : public QDialog, public Ui_TextEditorDialog
	{
		Q_OBJECT

	public:
        TextEditorDialog(QWidget* parent);
		virtual ~TextEditorDialog();

		// get selecting node
        static bool getText(QWidget* parent, Echo::String& expression, bool readOnly);
        
        // function name
        const Echo::String getPlainText() const;
        void setPlainText(const Echo::String& functionName);

        // readonly
        void setReadOnly(bool readOnly);
        bool isReadOnly() const;
        
    private slots:
        // on function name changed slot
        void onTextChanged();

    private:
        LuaSyntaxHighLighter*   m_luaSyntaxHighLighter = nullptr;
	};
}
