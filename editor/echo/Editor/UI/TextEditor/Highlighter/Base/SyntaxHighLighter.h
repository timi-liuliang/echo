#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <engine/core/util/StringUtil.h>

class QTextDocument;
namespace Studio
{
	class SyntaxHighLighter : public QSyntaxHighlighter
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
		SyntaxHighLighter(QTextDocument* parent = 0);

		// key words
		const QStringList& getKeyWords() const { return m_keyWords; }
        
        // append rule
        void appendForegroundRule( int r, int g, int b, const Echo::String& regExp, RuleGroup group=RG_Default);
        void appendBackgroundRule( int r, int g, int b, const Echo::String& regExp, RuleGroup group=RG_Default);
        
        // remove rule
        int removeRule(RuleGroup group);

	protected:
		// hight light block
		virtual void  highlightBlock( const QString& text) override;

	protected:
		// High light rule
		struct HighlightingRule
		{
            RuleGroup       group;
			QRegExp			pattern;
			QTextCharFormat format;
		};

		QStringList					m_keyWords;
		QRegExp						m_commentStartExpression;
		QRegExp						m_commentEndExpression;
		QTextCharFormat				m_multiLineCommentFormat;
		QVector<HighlightingRule>	m_highLightRules;
	};
}
