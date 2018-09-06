#pragma once

#include <QPlainTextEdit>
#include <QCompleter>
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class LuaTextEdit : public  QPlainTextEdit
	{
		Q_OBJECT

	public:
		LuaTextEdit(QWidget* parent = nullptr);
		virtual ~LuaTextEdit();

		void setModel(const QStringList& words);
		QCompleter* getCompleter() const;

	protected:
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void focusInEvent(QFocusEvent* e) override;

	private:
		// insert completion
		void insertCompletion(const QString& completion);

		// get text under cursor
		QString textUnderCursor() const;

		// current line
		Echo::String textCurrentLine() const;

		// auto indent
		void autoIndent(QKeyEvent* e);

	private:
		QStringList		m_keyWords;
		QCompleter*		m_completer;
	};
}