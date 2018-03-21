//#pragma once
//
//#include <QSyntaxHighlighter>
//#include <QTextCharFormat>
//
//class QTextDocument;
//namespace Studio
//{
//	//-------------------------------
//	// Lua高度编辑 2017-04-06
//	//-------------------------------
//	class LuaSyntaxHighLighter : public QSyntaxHighlighter
//	{
//		Q_OBJECT
//
//	public:
//		// 构造函数
//		LuaSyntaxHighLighter(QTextDocument* parent = 0);
//
//	protected:
//		// 高度block
//		void  highlightBlock( const QString& text);
//
//	private:
//		// 高度规则
//		struct HighlightingRule
//		{
//			QRegExp			pattern;
//			QTextCharFormat format;
//		};
//
//		QRegExp commentStartExpression;
//		QRegExp commentEndExpression;
//		QTextCharFormat classFormat;
//		QTextCharFormat singleLineCommentFormat;
//		QTextCharFormat multiLineCommentFormat;
//		QTextCharFormat quotationFormat;
//		QTextCharFormat functionFormat;
//		QTextCharFormat globalFormat;						// 全局变量
//		QTextCharFormat	mainFunFormat;	
//
//		QVector<HighlightingRule>	m_highLightRules;
//		QTextCharFormat				m_keyWordFormat;		// 关键字格式
//	};
//}