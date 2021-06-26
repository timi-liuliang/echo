#include <QtGui>
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	LuaSyntaxHighLighter::LuaSyntaxHighLighter(QTextDocument* parent)
		: SyntaxHighLighter( parent)
	{
		// set words
		m_keyWords << "self" << "function" << "if" << "then" << "for" << "return" << "end" << "do" << "pairs" << "ipairs";

		// function
		appendForegroundRule(97, 175, 239, "\\b[A-Za-z0-9_]+(?=\\()");

		// keywords
		appendForegroundRule(192, 120, 221, "\\blocal\\b");
		appendForegroundRule(192, 120, 221, "\\bfunction\\b");
		appendForegroundRule(192, 120, 221, "\\bend\\b");
		appendForegroundRule(192, 120, 221, "\\bif\\b");
		appendForegroundRule(192, 120, 221, "\\bthen\\b");
		appendForegroundRule(192, 120, 221, "\\belse\\b");
		appendForegroundRule(192, 120, 221, "\\bdo\\b");
		appendForegroundRule(192, 120, 221, "\\breturn\\b");
		appendForegroundRule(192, 120, 221, "\\bfor\\b");
		appendForegroundRule(192, 120, 221, "\\bwhile\\b");
		appendForegroundRule(192, 120, 221, "\\bin\\b");
        appendForegroundRule(192, 120, 221, "\\bbreak\\b");
		appendForegroundRule(192, 120, 221, "\\bor\\b");
		appendForegroundRule(192, 120, 221, "\\band\\b");

		// false true ...
		appendForegroundRule(209, 154, 102, "\\bnil\\b");
		appendForegroundRule(209, 154, 102, "\\btrue\\b");
		appendForegroundRule(209, 154, 102, "\\bfalse\\b");
		appendForegroundRule( 97, 175, 239, "\\bQ[A-Za-z]+\\b");

		// command
		appendForegroundRule(128, 138, 156, "--[^\n]*");

		// strings
		appendForegroundRule( 152, 195, 121, "\".*\"");

		// number
		appendForegroundRule(209, 154, 102, "\\b[0-9]+.?[0-9]+\\b");
		appendForegroundRule(209, 154, 102, "\\b[0-9]+\\b");

		m_commentStartExpression = QRegExp("--[[");
		m_commentEndExpression = QRegExp("]]--");
	}
}
