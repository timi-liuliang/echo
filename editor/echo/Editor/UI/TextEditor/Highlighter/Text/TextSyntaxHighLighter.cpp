#include <QtGui>
#include "TextSyntaxHighLighter.h"

namespace Studio
{
	TextSyntaxHighLighter::TextSyntaxHighLighter(QTextDocument* parent)
		: SyntaxHighLighter( parent)
	{
		// function
		appendForegroundRule(97, 175, 239, "\\b[A-Za-z0-9_]+(?=\\()");

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

		commentStartExpression = QRegExp("--[[");
		commentEndExpression = QRegExp("]]--");
	}
}
