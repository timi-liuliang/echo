#pragma once

#include <QDialog>
#include "ui_SettingChooseDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class SettingChooseDialog : public QDialog, public Ui_SettingChooseDialog
	{
		Q_OBJECT

	public:
		SettingChooseDialog(QWidget* parent);
		virtual ~SettingChooseDialog();

		// get setting
		static Echo::String getSelectingSetting(QWidget* parent);

	private:
		// update setting list
		void updateSettingList();

		// get selecting setting
		const Echo::String getSelectingSetting() const;
	};
}