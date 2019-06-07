#pragma once

#include <QDialog>
#include "ui_SlotChooseDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class SlotChooseDialog : public QDialog, public Ui_SlotChooseDialog
	{
		Q_OBJECT

	public:
		SlotChooseDialog(QWidget* parent);
		virtual ~SlotChooseDialog();

		// get selecting node
        static bool getSlot(QWidget* parent, Echo::String& nodePath, Echo::String& functionName);

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
