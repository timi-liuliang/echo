#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <engine/core/util/StringUtil.h>
#include "SyntaxHighLighter.h"

namespace Studio
{
	class TextSyntaxHighLighter : public SyntaxHighLighter
	{
		Q_OBJECT

	public:
		TextSyntaxHighLighter(QTextDocument* parent = 0);
	};
}
