#include "SettingChooseDialog.h"
#include <engine/core/base/class.h>

namespace Studio
{
	SettingChooseDialog::SettingChooseDialog(QWidget* parent)
		: QDialog(parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// init setting list widget
		updateSettingList();
	}

	SettingChooseDialog::~SettingChooseDialog()
	{

	}

	Echo::String SettingChooseDialog::getSelectingSetting(QWidget* parent)
	{
		static SettingChooseDialog dialog( parent);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			return dialog.getSelectingSetting();
		}
		else
		{
			return Echo::StringUtil::BLANK;
		}
	}

	// get selecting setting
	const Echo::String SettingChooseDialog::getSelectingSetting() const 
	{ 
		QListWidgetItem* item = m_settingListWidget->currentItem();
		if (item)
		{
			return item->text().toStdString().c_str();
		}

		return Echo::StringUtil::BLANK;
	}

	void SettingChooseDialog::updateSettingList()
	{
		m_settingListWidget->clear();

		Echo::StringArray classes;
		Echo::Class::getAllClasses(classes);
		for (Echo::String& className : classes)
		{
			if (Echo::Class::isSingleton(className))
			{
				m_settingListWidget->addItem( className.c_str());
			}
		}
	}
}