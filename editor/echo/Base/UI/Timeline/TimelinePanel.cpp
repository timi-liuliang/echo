#include "TimelinePanel.h"
#include "ui_TimelinePanel.h"
#include "QScrollBar"

namespace Studio
{
	static const int kUnitHeight = 17;
	static const QColor kLayerItemBgColor(220, 237, 201);
	TimelinePanel::TimelinePanel(QWidget *parent)
		: ITimelinePanel(parent)
		, m_ui(new Ui::TimelinePanel)
		, m_eventHandler(new ITimelinePanelEvent)
	{
		m_ui->setupUi(this);
		//m_ui->resumeButton->hide();
		connect(m_ui->ruler, SIGNAL(frameClicked(int)), this, SIGNAL(frameClicked(int)));
		connect(m_ui->editPanelScrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(on_editPanelScrollArea_verticalScroll(int)));
	    connect(m_ui->editPanelScrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)), m_ui->ruler, SLOT(on_editPanelScrollArea_horizontalScroll(int)));
	}

	TimelinePanel::~TimelinePanel()
	{
		delete m_ui;
	}

	void TimelinePanel::setTimePerFrame(unsigned int time)
	{
		m_ui->ruler->SetTimePerFrame(time);
		m_ui->editPanelScrollAreaWidgetContents->SetTimePerFrame(time);
	}

	uint TimelinePanel::getTimePerFrame()
	{
		return m_ui->ruler->getTimePerFrame();
	}

	void TimelinePanel::setRulerUsingFloat(bool b)
	{
		m_ui->ruler->setUsingFloat(b); 
	}

	void TimelinePanel::setLabelText(const QString& text)
	{
		m_ui->label->setText(text); 
	}

	void TimelinePanel::setItemCheckable(bool isCheckable)
	{
		m_isCheckable = isCheckable; 
		int count = m_ui->listWidget->count(); 
		for (int i = 0; i < count; i++)
		{
			auto* item = m_ui->listWidget->item(i); 
			auto flag = item->flags();

			flag = isCheckable ? flag |= Qt::ItemFlag::ItemIsUserCheckable
				: flag &~Qt::ItemFlag::ItemIsUserCheckable; 

			item->setFlags(flag);
		}
	}

	void TimelinePanel::addItem(const LineItem& item, const QString& icon)
	{
		auto listWidgetItem = new QListWidgetItem(item.name());
		
		listWidgetItem->setSizeHint(QSize(listWidgetItem->sizeHint().width(), kUnitHeight));
		listWidgetItem->setBackgroundColor(kLayerItemBgColor);
		
		if (m_isCheckable)
		{
			listWidgetItem->setCheckState(item.checkState());
		}

		listWidgetItem->setTextAlignment(Qt::AlignLeft);
		listWidgetItem->setIcon(QIcon(icon));

		m_ui->listWidget->addItem(listWidgetItem);
		m_ui->editPanelScrollAreaWidgetContents->addItem(item);
	}

	EditPanel* TimelinePanel::getEditPanel() const
	{
		return m_ui->editPanelScrollAreaWidgetContents; 
	}

	int TimelinePanel::getRulerOffset() const
	{
		return m_ui->ruler->getRulerOffset(); 
	}

	LineItem TimelinePanel::item(int index)
	{
		auto* listWidgetItem = m_ui->listWidget->item(index); 
		if (!listWidgetItem)
		{
			return LineItem(); 
		}

		LineItem item; 
		item.setName(listWidgetItem->text()); 
		item.setCheckState(listWidgetItem->checkState()); 

		return item; 
	}

	void TimelinePanel::UpdateUIKeyFramePos(int layerindex, int frameIndex, int offset)
	{
		m_ui->editPanelScrollAreaWidgetContents->UpdateUIKeyFramePos(layerindex,frameIndex,offset);
	}

	void TimelinePanel::setEventHandler(ITimelinePanelEvent* eventHandler)
	{
		m_eventHandler.reset(eventHandler ? eventHandler : new ITimelinePanelEvent);
	}

	void TimelinePanel::clear()
	{
		m_ui->listWidget->clear();
		m_ui->editPanelScrollAreaWidgetContents->clear();
	}

	void TimelinePanel::UpdateVertTimeFrame(int time)
	{
		m_ui->ruler->UpdateVertTimeFrame(time);
		m_ui->editPanelScrollAreaWidgetContents->UpdateVertTimeFrame(time);
	}

	void TimelinePanel::on_playButton_clicked()
	{
		onButtonClicked(ITimelinePanelEvent::PlayButton);
	}

	void TimelinePanel::on_pauseButton_clicked()
	{
		//m_ui->pauseButton->hide();
		//m_ui->resumeButton->show();
		onButtonClicked(ITimelinePanelEvent::PauseButton);
	}

	void TimelinePanel::on_resumeButton_clicked()
	{
		//m_ui->resumeButton->hide();
		//m_ui->pauseButton->show();
		onButtonClicked(ITimelinePanelEvent::ResumeButton);
	}

	void TimelinePanel::on_stopButton_clicked()
	{
		onButtonClicked(ITimelinePanelEvent::StopButton);
	}

	void TimelinePanel::on_stepButton_clicked()
	{
		onButtonClicked(ITimelinePanelEvent::StepButton);
	}

	void TimelinePanel::on_listWidget_itemClicked(QListWidgetItem* item)
	{
		int row = m_ui->listWidget->row(item);
		m_eventHandler->onItemClicked(row);
		m_ui->editPanelScrollAreaWidgetContents->setSelectedLayer(row);
	}

	void TimelinePanel::on_listWidget_verticalScroll(int val)
	{
		m_ui->editPanelScrollArea->verticalScrollBar()->setValue(val);
	}

	void TimelinePanel::on_listWidget_itemChanged(QListWidgetItem* item)
	{
		auto row = m_ui->listWidget->row(item);
		m_eventHandler->onItemChecked(row, item->checkState());
	}

	void TimelinePanel::on_listWidget_customContextMenuRequested(const QPoint& p)
	{
		auto item = m_ui->listWidget->itemAt(p);
		auto row = m_ui->listWidget->row(item);
		m_eventHandler->onCustomContextMenuRequested(row);
	}

	void TimelinePanel::on_editPanelScrollArea_verticalScroll(int value)
	{
		m_ui->listWidget->verticalScrollBar()->setValue(value);
	}

	void TimelinePanel::onEventButtonMoved(int row, int index, float tick)
	{
		m_eventHandler->onEventButtonMoved(row, index, tick);
	}

	void TimelinePanel::onEventButtonClicked(int row, int index, int subindex)
	{
		m_eventHandler->onEventButtonClicked(row, index, subindex);
	}

	void TimelinePanel::onFrameMoveto(float tick)
	{
		m_eventHandler->onFrameMoveto(tick);
	}

	void TimelinePanel::onCustomContextMenuRequested(int row)
	{
		m_eventHandler->onCustomContextMenuRequested(row);
	}

	void TimelinePanel::onCustomContextMenuRequested(int row, int column)
	{
		m_eventHandler->onCustomContextMenuRequested(row, column);
	}

	void TimelinePanel::onButtonClicked(ITimelinePanelEvent::Button button)
	{
		auto count = m_ui->listWidget->count();
		for (auto i = 0; i < count; ++i)
		{
			auto item = m_ui->listWidget->item(i);
			if (item->checkState() == Qt::Checked)
			{
				m_eventHandler->onButtonClicked(button, item->text());
			}
		}
	}


}  // namespace Studio
