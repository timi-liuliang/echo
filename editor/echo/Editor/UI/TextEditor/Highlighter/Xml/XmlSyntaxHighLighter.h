#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <engine/core/util/StringUtil.h>
#include "SyntaxHighLighter.h"

namespace Studio
{
	class XmlSyntaxHighLighter : public SyntaxHighLighter
	{
		Q_OBJECT

	public:
		XmlSyntaxHighLighter(QTextDocument* parent = 0);
	};
}
