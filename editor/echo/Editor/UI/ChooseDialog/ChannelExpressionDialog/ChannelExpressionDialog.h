#pragma once

#include <QDialog>
#include "ui_ChannelExpressionDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class ChannelExpressionDialog : public QDialog, public Ui_ChannelExpressionDialog
	{
		Q_OBJECT

	public:
		ChannelExpressionDialog(QWidget* parent);
		virtual ~ChannelExpressionDialog();

		// get selecting node
        static bool getExpression(QWidget* parent, Echo::String& expression);

		// get selecting node path
		const Echo::String getSelectingNodePath() const;
        
        // function name
        const Echo::String getFunctionName() const;
        void setFunctionName(const Echo::String& functionName);
        
    private slots:
        // on function name changed slot
        void onFunctionNameChanged();
	};
}
