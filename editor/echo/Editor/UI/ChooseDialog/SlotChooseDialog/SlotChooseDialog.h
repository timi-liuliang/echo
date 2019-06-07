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
		static Echo::String getSelectingNode( QWidget* parent);

	private:
		// get selecting node path
		const Echo::String getSelectingNodePath() const;
	};
}
