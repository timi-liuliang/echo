#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <engine/core/util/StringUtil.h>
#include "SyntaxHighLighter.h"

class QTextDocument;
namespace Studio
{
	class LuaSyntaxHighLighter : public SyntaxHighLighter
	{
		Q_OBJECT

	public:
		LuaSyntaxHighLighter(QTextDocument* parent = 0);
	};
}
