#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <engine/core/util/StringUtil.h>
#include "SyntaxHighLighter.h"

namespace Studio
{
	class GLSLSyntaxHighLighter : public SyntaxHighLighter
	{
		Q_OBJECT

	public:
		GLSLSyntaxHighLighter(QTextDocument* parent = 0);
	};
}
