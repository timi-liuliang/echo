#include <QtGui>
#include "SyntaxHighLighter.h"

namespace Studio
{
	SyntaxHighLighter::SyntaxHighLighter(QTextDocument* parent)
		: QSyntaxHighlighter( parent)
	{
	}

	void SyntaxHighLighter::appendForegroundRule(int r, int g, int b, const Echo::String& regExp, RuleGroup group)
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

	void SyntaxHighLighter::appendBackgroundRule(int r, int g, int b, const Echo::String& regExp, RuleGroup group)
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
    
    int SyntaxHighLighter::removeRule(RuleGroup group)
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

	void  SyntaxHighLighter::highlightBlock(const QString& text)
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
			startIndex = m_commentStartExpression.indexIn(text);

		while (startIndex >= 0) 
		{
			int endIndex = m_commentEndExpression.indexIn(text, startIndex);
			int commentLength;
			if (endIndex == -1) 
			{
				setCurrentBlockState(1);
				commentLength = text.length() - startIndex;
			} else 
			{
				commentLength = endIndex - startIndex + m_commentEndExpression.matchedLength();
			}

			QColor color; color.setRgb( 92, 99, 112);
			m_multiLineCommentFormat.setForeground(color);
			setFormat(startIndex, commentLength, m_multiLineCommentFormat);
			startIndex = m_commentStartExpression.indexIn(text, startIndex + commentLength);
		}
	}
}
