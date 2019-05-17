#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <engine/core/util/StringUtil.h>

class QTextDocument;
namespace Studio
{
	class LuaSyntaxHighLighter : public QSyntaxHighlighter
	{
		Q_OBJECT
        
    public:
        // group
        enum RuleGroup
        {
            RG_Default = 0,
            RG_SelectTextBlock = 1,
        };

	public:
		LuaSyntaxHighLighter(QTextDocument* parent = 0);
        
        // append rule
        void appendForegroundRule( int r, int g, int b, const Echo::String& regExp, RuleGroup group=RG_Default);
        void appendBackgroundRule( int r, int g, int b, const Echo::String& regExp, RuleGroup group=RG_Default);
        
        // remove rule
        int removeRule(RuleGroup group);

	protected:
		// hight light block
		virtual void  highlightBlock( const QString& text) override;

	private:
		// High light rule
		struct HighlightingRule
		{
            RuleGroup       group;
			QRegExp			pattern;
			QTextCharFormat format;
		};

		QRegExp						commentStartExpression;
		QRegExp						commentEndExpression;
		QTextCharFormat				multiLineCommentFormat;
		QTextCharFormat				singleLineCommentFormat;
		QVector<HighlightingRule>	m_highLightRules;
	};
}
