#pragma once

#include <QTextEdit>
#include <QCompleter>

namespace Studio
{
	class LuaTextEdit : public QTextEdit
	{
		Q_OBJECT

	public:
		LuaTextEdit(QWidget* parent = nullptr);
		virtual ~LuaTextEdit();

		void setCompleter(QCompleter* c);
		QCompleter* getCompleter() const;

	protected:
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void focusInEvent(QFocusEvent* e) override;

	private:
		void insertCompletion(const QString& completion);

	private:
		QString textUnderCursor() const;

	private:
		QCompleter*		m_completer;
	};
}