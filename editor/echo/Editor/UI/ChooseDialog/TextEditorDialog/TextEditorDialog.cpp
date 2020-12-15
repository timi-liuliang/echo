#include "TextEditorDialog.h"
#include "NodeTreePanel.h"
#include "ReferenceChooseDialog.h"
#include <QStatusBar>

namespace Studio
{
	TextEditorDialog::TextEditorDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		QStatusBar* statusBar = new QStatusBar(this);
		verticalLayoutRoot->addWidget(statusBar);

#ifdef ECHO_PLATFORM_WINDOWS
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		m_menuBar->setNativeMenuBar(false);
#endif

		m_menuBar->setCornderButtonVisible(QT_UI::QMenuBarEx::FullScreen, false);

		// syntax high lighter
		m_luaSyntaxHighLighter = new LuaSyntaxHighLighter(m_textEdit->document());
		m_textEdit->setSyntaxHighter(m_luaSyntaxHighLighter);
        
        // connect signal slot
        QObject::connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	}

	TextEditorDialog::~TextEditorDialog()
	{

	}

	bool TextEditorDialog::getText(QWidget* parent, Echo::String& text, bool readOnly)
	{
		TextEditorDialog dialog(parent);
        dialog.setPlainText(text);
		dialog.setReadOnly(readOnly);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			text = dialog.getPlainText();
            
            return true;
		}
		else
		{
			return false;
		}
	}
    
    const Echo::String TextEditorDialog::getPlainText() const
    {
        return m_textEdit->toPlainText().toStdString().c_str();
    }
    
    void TextEditorDialog::setPlainText(const Echo::String& functionName)
    {
		m_textEdit->setPlainText(functionName.c_str());
    }

	void TextEditorDialog::setReadOnly(bool readOnly)
	{
		m_textEdit->setReadOnly(readOnly);
	}

	bool TextEditorDialog::isReadOnly() const
	{
		return m_textEdit->isReadOnly();
	}
    
    void TextEditorDialog::onTextChanged()
    {
        Echo::String expression = getPlainText();
        m_ok->setEnabled(!expression.empty());
    }
}
