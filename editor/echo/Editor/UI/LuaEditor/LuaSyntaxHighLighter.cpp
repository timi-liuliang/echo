#include <QtGui>
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	// 构造函数
	LuaSyntaxHighLighter::LuaSyntaxHighLighter(QTextDocument* parent)
		: QSyntaxHighlighter( parent)
	{
		// function
		appendRule(97, 175, 239, "\\b[A-Za-z0-9_]+(?=\\()");

		// keywords
		appendRule(192, 120, 221, "\\blocal\\b");
		appendRule(192, 120, 221, "\\bfunction\\b");
		appendRule(192, 120, 221, "\\bend\\b");
		appendRule(192, 120, 221, "\\bif\\b");
		appendRule(192, 120, 221, "\\bthen\\b");
		appendRule(192, 120, 221, "\\belse\\b");
		appendRule(192, 120, 221, "\\bdo\\b");
		appendRule(192, 120, 221, "\\breturn\\b");
		appendRule(192, 120, 221, "\\bfor\\b");
		appendRule(192, 120, 221, "\\bin\\b");

		// false true ...
		appendRule(209, 154, 102, "\\bnil\\b");
		appendRule(209, 154, 102, "\\btrue\\b");
		appendRule(209, 154, 102, "\\bfalse\\b");
		appendRule( 97, 175, 239, "\\bQ[A-Za-z]+\\b");


		// 注释
		appendRule(128, 138, 156, "--[^\n]*");

		// 字符串
		appendRule( 152, 195, 121, "\".*\"");

		// 数字
		appendRule(209, 154, 102, "\\b[0-9]+.?[0-9]+\\b");
		appendRule(209, 154, 102, "\\b[0-9]+\\b");

		commentStartExpression = QRegExp("--[[");
		commentEndExpression = QRegExp("]]--");
	}

	// append rule
	void LuaSyntaxHighLighter::appendRule(int r, int g, int b, const Echo::String& regExp)
	{
		QColor color;
		color.setRgb(r, g, b);

		HighlightingRule rule;
		rule.format.setForeground(color);
		rule.pattern = QRegExp(regExp.c_str());

		m_highLightRules.append(rule);
	}

	// 高度block
	void  LuaSyntaxHighLighter::highlightBlock(const QString& text)
	{
		foreach (const HighlightingRule &rule, m_highLightRules) 
		{
			QRegExp expression(rule.pattern);
			int index = expression.indexIn(text);

			while (index >= 0) 
			{
				int length = expression.matchedLength();
				setFormat(index, length, rule.format);
				index = expression.indexIn(text, index + length);
			}
		}

		setCurrentBlockState(0);

		int startIndex = 0;
		if (previousBlockState() != 1)
			startIndex = commentStartExpression.indexIn(text);

		while (startIndex >= 0) 
		{
			int endIndex = commentEndExpression.indexIn(text, startIndex);
			int commentLength;
			if (endIndex == -1) 
			{
				setCurrentBlockState(1);
				commentLength = text.length() - startIndex;
			} else 
			{
				commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
			}

			QColor color; color.setRgb( 92, 99, 112);
			multiLineCommentFormat.setForeground(color);
			setFormat(startIndex, commentLength, multiLineCommentFormat);
			startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
		}
	}
}