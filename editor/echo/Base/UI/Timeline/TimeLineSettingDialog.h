#pragma once
#include <QDialog>

namespace Ui {
	class TimeLineSettingDialog;
}

namespace Studio
{
	class AStudio;
	class TimeLineSettingDialog : public QDialog
	{
		Q_OBJECT
	public:
		TimeLineSettingDialog(QWidget *parent, AStudio* studio);
		~TimeLineSettingDialog();

		virtual void accept() Q_DECL_OVERRIDE;

	private:
		Ui::TimeLineSettingDialog* m_ui;
		AStudio* m_studio;
	};
}  // namespace Studio
