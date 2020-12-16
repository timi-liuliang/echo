#include <QtGui>
#include "GlslSyntaxHighLighter.h"

namespace Studio
{
	GLSLSyntaxHighLighter::GLSLSyntaxHighLighter(QTextDocument* parent)
		: SyntaxHighLighter( parent)
	{
		// set words
		m_keyWords << "void" << "function" << "if" << "then" << "for" << "return" << "end" << "do" << "pairs" << "ipairs";
		m_keyWords << "float" << "vec2" << "vec3" << "vec4";

		// function
		appendForegroundRule(97, 175, 239, "\\b[A-Za-z0-9_]+(?=\\()");

		// keywords
		appendForegroundRule(192, 120, 221, "\\bfloat\\b");
		appendForegroundRule(192, 120, 221, "\\bvec2\\b");
		appendForegroundRule(192, 120, 221, "\\bvec3\\b");
		appendForegroundRule(192, 120, 221, "\\bvec4\\b");
		appendForegroundRule(192, 120, 221, "\\bfunction\\b");
		appendForegroundRule(192, 120, 221, "\\bif\\b");
		appendForegroundRule(192, 120, 221, "\\belse\\b");
		appendForegroundRule(192, 120, 221, "\\bdo\\b");
		appendForegroundRule(192, 120, 221, "\\breturn\\b");
		appendForegroundRule(192, 120, 221, "\\bfor\\b");
		appendForegroundRule(192, 120, 221, "\\bwhile\\b");
		appendForegroundRule(192, 120, 221, "\\bin\\b");
		appendForegroundRule(192, 120, 221, "\\bout\\b");
        appendForegroundRule(192, 120, 221, "\\bbreak\\b");

		// false true ...
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
