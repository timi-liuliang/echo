#include <QtGui>
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	LuaSyntaxHighLighter::LuaSyntaxHighLighter(QTextDocument* parent)
		: QSyntaxHighlighter( parent)
	{
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
		appendForegroundRule(192, 120, 221, "\\bin\\b");

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

	void LuaSyntaxHighLighter::appendForegroundRule(int r, int g, int b, const Echo::String& regExp, RuleGroup group)
	{
		QColor color;
		color.setRgb(r, g, b);

		HighlightingRule rule;
        rule.group = group;
		rule.format.setForeground(color);
		rule.pattern = QRegExp(regExp.c_str());

		m_highLightRules.append(rule);
        
        // update display immediately
        //rehighlight();
	}

	void LuaSyntaxHighLighter::appendBackgroundRule(int r, int g, int b, const Echo::String& regExp, RuleGroup group)
	{
		QColor color;
		color.setRgb(r, g, b);

		HighlightingRule rule;
        rule.group = group;
		rule.format.setBackground(color);
		rule.pattern = QRegExp(regExp.c_str());

		m_highLightRules.append(rule);
        
        // update display immediately
        //rehighlight();
	}
    
    int LuaSyntaxHighLighter::removeRule(RuleGroup group)
    {
        int removeCount = 0;
        for(QVector<HighlightingRule>::iterator it=m_highLightRules.begin(); it!=m_highLightRules.end();)
        {
            if(it->group == group)
            {
                it = m_highLightRules.erase(it);
                removeCount++;
            }
            else
                it++;
        }
        
        return removeCount;
    }

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
