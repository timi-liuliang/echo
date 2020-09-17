#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "LuaEditor.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

namespace Studio
{
	LuaEditor::LuaEditor(QWidget* parent)
		: QWidget(parent)
	{
		setupUi( this);

		// syntax high lighter
		m_luaSyntaxHighLighter = new LuaSyntaxHighLighter(m_plainTextEdit->document());
        m_plainTextEdit->setSyntaxHighter(m_luaSyntaxHighLighter);

		// connections
		QObject::connect(m_plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	}

	LuaEditor::~LuaEditor()
	{
	}

	void LuaEditor::open(const Echo::String& fullPath)
	{
		m_luaRes = ECHO_DOWN_CAST<Echo::LuaScript*>(Echo::Res::get(fullPath));
		if (m_luaRes)
		{
			m_plainTextEdit->setPlainText(m_luaRes->getSrc());

			// change title
			updateTitle();
		}
	}

	const Echo::String& LuaEditor::getCurrentLuaFilePath()
	{
		if (m_luaRes)
		{
			return m_luaRes->getPath();
		}

		return Echo::StringUtil::BLANK;
	}

	// on text changed
	void LuaEditor::onTextChanged()
	{
		if (m_luaRes)
		{
			updateTitle();

			m_luaRes->setSrc(m_plainTextEdit->toPlainText().toStdString().c_str());
		}
	}

	void LuaEditor::save()
	{
		if (m_luaRes)
		{
			m_luaRes->save();

			// change title
			updateTitle();
		}
	}

	void LuaEditor::updateTitle()
	{
		if (m_luaRes)
		{
			if (m_luaRes->getSrc() == m_plainTextEdit->toPlainText())
			{
				Echo::String fileName = Echo::PathUtil::GetPureFilename(m_luaRes->getPath());
				setWindowTitle( fileName.c_str());
			}
			else
			{
				Echo::String fileName = Echo::PathUtil::GetPureFilename(m_luaRes->getPath());
				setWindowTitle((fileName + "*").c_str());
			}
		}
		else
		{
			setWindowTitle("LuaEditor");
		}
        
        emit titleChanged(this);
	}
}
