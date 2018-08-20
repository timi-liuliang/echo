#include "ProjectMgr.h"
#include <QFileDialog>
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
		//QObject::connect(m_actionSave, SIGNAL(triggered()), this, SLOT(save()));
		QObject::connect(m_textEditor, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	}

	// 析构函数
	LuaEditor::~LuaEditor()
	{
	}

	// 显示纹理
	void LuaEditor::open(const Echo::String& fullPath)
	{
		m_fullPath = "";

		// 1.显示Lua
		QFile file(fullPath.c_str());
		if (file.open(QFile::ReadOnly | QFile::Text))
		{
			m_fullPath = fullPath;
			m_origContent = file.readAll();
			m_textEditor->setPlainText(m_origContent);

			file.close();
		}

		// 0.更改标题
		updateTitle();
	}

	// 内容被修改
	void LuaEditor::onTextChanged()
	{
		// 0.更改标题
		updateTitle();
	}

	// 保存
	void LuaEditor::save()
	{
		if (!m_fullPath.empty())
		{
			QFile file(m_fullPath.c_str());
			if (file.open(QFile::WriteOnly | QFile::Text))
			{
				m_origContent = m_textEditor->toPlainText();
				file.write(m_origContent.toUtf8());

				file.flush();
				file.close();
			}

			// 0.更改标题
			updateTitle();
		}
	}

	// 更新标题显示
	void LuaEditor::updateTitle()
	{
		if (m_origContent == m_textEditor->toPlainText())
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(m_fullPath);
			this->setWindowTitle( fileName.empty() ? "LuaEditor" : fileName.c_str());
		}
		else
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(m_fullPath);
			this->setWindowTitle(fileName.empty() ? "LuaEditor" :(fileName+"*").c_str());
		}
	}
}