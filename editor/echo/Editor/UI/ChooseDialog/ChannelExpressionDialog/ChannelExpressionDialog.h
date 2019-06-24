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
        
        // function name
        const Echo::String getExpressionText() const;
        void setExpressionText(const Echo::String& functionName);
        
    private slots:
        // on function name changed slot
        void onExpressionChanged();
	};
}
