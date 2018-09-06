#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "LuaEditor.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

namespace Studio
{
	LuaEditor::LuaEditor(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi( this);

		// font
		QFont font;
		font.setFamily("Courier");
		font.setStyleHint(QFont::Monospace);
		font.setFixedPitch(true);
		font.setPointSize(10);
		font.setBold(true);
		m_plainTextEdit->setFont(font);

		// Tab Space
		const int tabStop = 4;  // 4 characters
		QFontMetrics metrics(font);
		m_plainTextEdit->setTabStopWidth(tabStop * metrics.width(' '));

		// syntax high lighter
		m_luaSyntaxHighLighter = new LuaSyntaxHighLighter(m_plainTextEdit->document());

		// connections
		QObject::connect(m_plainTextEdit, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	}

	LuaEditor::~LuaEditor()
	{
	}

	// size hint
	QSize LuaEditor::sizeHint() const
	{
		float width = MainWindow::instance()->width() * 0.7f;
		float height = MainWindow::instance()->height() * 0.65f;
		return QSize(width, height);
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

	// get current edit lua file path
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

	// update title display
	void LuaEditor::updateTitle()
	{
		if (m_luaRes)
		{
			if (m_luaRes->getSrc() == m_plainTextEdit->toPlainText())
			{
				Echo::String fileName = m_luaRes->getPath();
				setWindowTitle( fileName.c_str());
			}
			else
			{
				Echo::String fileName = m_luaRes->getPath();
				setWindowTitle((fileName + "*").c_str());
			}
		}
		else
		{
			setWindowTitle("LuaEditor");
		}
	}
}