//#include <QtGui>
//#include "LuaSyntaxHighLighter.h"
//
//namespace Studio
//{
//	// 构造函数
//	LuaSyntaxHighLighter::LuaSyntaxHighLighter(QTextDocument* parent)
//		: QSyntaxHighlighter( parent)
//	{
//		HighlightingRule rule;
//
//		//functionFormat.setFontItalic(true);
//		functionFormat.setForeground(Qt::darkMagenta);
//		rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
//		rule.format = functionFormat;
//		m_highLightRules.append(rule);
//
//		QColor lightBlue; lightBlue.setRgb(14, 99, 156);
//		m_keyWordFormat.setForeground( lightBlue);
//		QStringList keywordPatterns;
//		keywordPatterns << "\\bfunction\\b"  << "\\bend\\b";
//
//		foreach( const QString& pattern, keywordPatterns)
//		{
//			rule.pattern = QRegExp( pattern);
//			rule.format  = m_keyWordFormat;
//			m_highLightRules.append( rule);
//		}
//
//		classFormat.setForeground(Qt::darkMagenta);
//		rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
//		rule.format = classFormat;
//		m_highLightRules.append(rule);
//
//		singleLineCommentFormat.setForeground(Qt::darkGreen);
//		rule.pattern = QRegExp("--[^\n]*");
//		rule.format = singleLineCommentFormat;
//		m_highLightRules.append(rule);
//
//		multiLineCommentFormat.setForeground(Qt::darkGreen);
//
//		// 字符串
//		QColor darkYellow; darkYellow.setRgb(173, 73, 30);
//		quotationFormat.setForeground(darkYellow);
//		rule.pattern = QRegExp("\".*\"");
//		rule.format = quotationFormat;
//		m_highLightRules.append(rule);
//
//		commentStartExpression = QRegExp("/\\*");
//		commentEndExpression = QRegExp("\\*/");
//	}
//
//	// 高度block
//	void  LuaSyntaxHighLighter::highlightBlock(const QString& text)
//	{
//		foreach (const HighlightingRule &rule, m_highLightRules) 
//		{
//			QRegExp expression(rule.pattern);
//			int index = expression.indexIn(text);
//
//			while (index >= 0) 
//			{
//				int length = expression.matchedLength();
//				setFormat(index, length, rule.format);
//				index = expression.indexIn(text, index + length);
//			}
//		}
//
//		setCurrentBlockState(0);
//
//		int startIndex = 0;
//		if (previousBlockState() != 1)
//			startIndex = commentStartExpression.indexIn(text);
//
//		while (startIndex >= 0) 
//		{
//			int endIndex = commentEndExpression.indexIn(text, startIndex);
//			int commentLength;
//			if (endIndex == -1) 
//			{
//				setCurrentBlockState(1);
//				commentLength = text.length() - startIndex;
//			} else 
//			{
//				commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
//			}
//
//			setFormat(startIndex, commentLength, multiLineCommentFormat);
//			startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
//		}
//	}
//}