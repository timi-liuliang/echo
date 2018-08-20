#include <QtGui>
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	// 构造函数
	LuaSyntaxHighLighter::LuaSyntaxHighLighter(QTextDocument* parent)
		: QSyntaxHighlighter( parent)
	{
		// atom lua
		HighlightingRule rule;

		// functionFormat.setFontItalic(true);
		QColor color; color.setRgb(97, 175, 239);
		functionFormat.setForeground(color);
		rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
		rule.format = functionFormat;
		m_highLightRules.append(rule);

		// keywords
		color.setRgb(192, 120, 221);
		m_keyWordFormat.setForeground(color);
		QStringList keywordPatterns;
		keywordPatterns << "\\blocal\\b" << "\\bfunction\\b"  << "\\bend\\b" << "\\bif\\b" << "\\bthen\\b" << "\\bdo\\b" << "\\breturn\\b" << "\\bfor\\b" << "\\bin\\b";
		foreach( const QString& pattern, keywordPatterns)
		{
			rule.pattern = QRegExp( pattern);
			rule.format  = m_keyWordFormat;
			m_highLightRules.append( rule);
		}

		// false true ...
		color.setRgb(209, 154, 102);
		m_keyWordFormat.setForeground(color);
		keywordPatterns.clear();
		keywordPatterns << "\\btrue\\b" << "\\bfalse\\b";
		foreach(const QString& pattern, keywordPatterns)
		{
			rule.pattern = QRegExp(pattern);
			rule.format = m_keyWordFormat;
			m_highLightRules.append(rule);
		}

		color.setRgb( 97, 175, 239);
		classFormat.setForeground(color);
		rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
		rule.format = classFormat;
		m_highLightRules.append(rule);

		// 注释
		color.setRgb(92, 99, 112);
		singleLineCommentFormat.setForeground( color);
		rule.pattern = QRegExp("--[^\n]*");
		rule.format = singleLineCommentFormat;
		m_highLightRules.append(rule);

		multiLineCommentFormat.setForeground(Qt::green);

		// 字符串
		color.setRgb(152, 195, 121);
		quotationFormat.setForeground(color);
		rule.pattern = QRegExp("\".*\"");
		rule.format = quotationFormat;
		m_highLightRules.append(rule);

		commentStartExpression = QRegExp("/\\*");
		commentEndExpression = QRegExp("\\*/");
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

			setFormat(startIndex, commentLength, multiLineCommentFormat);
			startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
		}
	}
}