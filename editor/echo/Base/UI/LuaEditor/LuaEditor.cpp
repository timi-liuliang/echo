#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "LuaEditor.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

namespace Studio
{
	// 构造函数
	LuaEditor::LuaEditor(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi( this);

		// 字体
		QFont font;
		font.setFamily("Courier");
		font.setStyleHint(QFont::Monospace);
		font.setFixedPitch(true);
		font.setPointSize(10);
		font.setBold(true);
		m_textEditor->setFont(font);

		// Tab Space
		const int tabStop = 4;  // 4 characters
		QFontMetrics metrics(font);
		m_textEditor->setTabStopWidth(tabStop * metrics.width(' '));

		// 语法高亮器
		m_luaSyntaxHighLighter = new LuaSyntaxHighLighter(m_textEditor->document());

		// 消息链接
		QObject::connect(m_textEditor, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	}

	// 析构函数
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

	// 显示纹理
	void LuaEditor::open(const Echo::String& fullPath)
	{
		m_luaRes = ECHO_DOWN_CAST<Echo::LuaScript*>(Echo::Res::get(fullPath));
		if (m_luaRes)
		{
			m_textEditor->setPlainText(m_luaRes->getSrc());

			// 0.更改标题
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

	// 内容被修改
	void LuaEditor::onTextChanged()
	{
		if (m_luaRes)
		{
			m_luaRes->setSrc( m_textEditor->toPlainText().toStdString().c_str());

			updateTitle();
		}
	}

	// 保存
	void LuaEditor::save()
	{
		if (m_luaRes)
		{
			m_luaRes->save();

			// 0.更改标题
			updateTitle();
		}
	}

	// 更新标题显示
	void LuaEditor::updateTitle()
	{
		if (m_luaRes)
		{
			if (m_luaRes->getSrc() == m_textEditor->toPlainText())
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