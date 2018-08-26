#include "LuaTextEdit.h"
#include <QTextCursor>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>

namespace Studio
{
	LuaTextEdit::LuaTextEdit(QWidget* parent)
		: QTextEdit(parent)
		, m_completer(nullptr)
	{
		QCompleter* completer = new QCompleter(this);
		completer = new QCompleter(this);

		QStringList words; words << "self" << "setLocalPosition";
		QStringListModel* model = new QStringListModel(words, completer);

		completer->setModel( model);
		completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		completer->setWrapAround(false);
		setCompleter(completer);
	}

	LuaTextEdit::~LuaTextEdit()
	{

	}

	void LuaTextEdit::setCompleter(QCompleter* c)
	{
		if (m_completer)
			QObject::disconnect(m_completer, 0, this, 0);

		m_completer = c;
		if (!m_completer)
			return;

		m_completer->setWidget(this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setCaseSensitivity(Qt::CaseInsensitive);
		
		// connect signal slot
		QObject::connect(m_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
	}

	QCompleter* LuaTextEdit::getCompleter() const
	{
		return m_completer;
	}

	void LuaTextEdit::keyPressEvent(QKeyEvent* e) 
	{
		if (m_completer && m_completer->popup()->isVisible())
		{
			switch (e->key())
			{
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_Tab:
			case Qt::Key_Backtab:	e->ignore();	return;
			default:								break;
			}
		}

		bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E);
		if (!m_completer || !isShortcut)
			QTextEdit::keyPressEvent(e);

		// We also handle other modifiers and shortcuts for which we do not want the completer 
		// to respond to.
		const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
		if (m_completer && (ctrlOrShift && e->text().isEmpty()))
			return;

		// end of word
		static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
		bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
		QString completionPrefix = textUnderCursor();

		if (!isShortcut && (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3 || eow.contains(e->text().right(1)))) 
		{
			m_completer->popup()->hide();
			return;
		}

		if (completionPrefix != m_completer->completionPrefix())
		{
			m_completer->setCompletionPrefix(completionPrefix);
			m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
		}

		// popup it up!
		QRect cr = cursorRect();
		cr.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
		m_completer->complete(cr); 
	}

	void LuaTextEdit::focusInEvent(QFocusEvent* e)
	{
		if (m_completer)
			m_completer->setWidget(this);

		QTextEdit::focusInEvent(e);
	}

	void LuaTextEdit::insertCompletion(const QString& completion)
	{
		if (m_completer->widget() != this)
			return;

		QTextCursor tc = textCursor();
		int extra = completion.length() - m_completer->completionPrefix().length();
		tc.movePosition(QTextCursor::Left);
		tc.movePosition(QTextCursor::EndOfWord);
		tc.insertText(completion.right(extra));
		setTextCursor(tc);
	}

	QString LuaTextEdit::textUnderCursor() const
	{
		QTextCursor tc = textCursor();
		tc.select(QTextCursor::WordUnderCursor);
		
		return tc.selectedText();
	}
}