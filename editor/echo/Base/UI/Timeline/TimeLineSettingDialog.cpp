#include "TimeLineSettingDialog.h"
#include "ui_TimeLineSettingDialog.h"
#include "Studio.h"
#include "ConfigMgr.h"
#include "TimelinePanel.h"

namespace Studio
{
	TimeLineSettingDialog::TimeLineSettingDialog(QWidget *parent, AStudio* studio)
		: QDialog(parent)
		, m_ui(new Ui::TimeLineSettingDialog)
		, m_studio(studio)
	{
		m_ui->setupUi(this);
		//uint value = m_studio->getTimelinePanel()->getTimePerFrame();
		//m_ui->framePerSecondSpinBox->setValue(int(1000 / value));
	}

	TimeLineSettingDialog::~TimeLineSettingDialog()
	{
		delete m_ui;
	}

	void TimeLineSettingDialog::accept()
	{
		QString text = m_ui->framePerSecondSpinBox->text();
		uint value = text.toUInt();
		if (value >= 10 && value <= 100)
		{
			//m_studio->getTimelinePanel()->setTimePerFrame(1000 / value);
			ConfigMgr* configMgr = AStudio::Instance()->getConfigMgr();
			configMgr->SetValue("BattleGlobalFPS", Echo::StringUtil::ToString(value).c_str());
			QDialog::accept();
		}
	}
}
