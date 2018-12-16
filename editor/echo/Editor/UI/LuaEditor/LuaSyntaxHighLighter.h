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
		LuaSyntaxHighLighter(QTextDocument* parent = 0);

	protected:
		// hight light block
		virtual void  highlightBlock( const QString& text) override;

		// append rule
		void appendRule( int r, int g, int b, const Echo::String& regExp);

	private:
		// 高度规则
		struct HighlightingRule
		{
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