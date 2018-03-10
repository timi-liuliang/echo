#pragma once

#include <QWidget>
#include <QScopedPointer>
#include "ITimelinePanel.h"
#include "ITimelinePanelEvent.h"

class QListWidgetItem;

namespace Ui {
	class TimelinePanel;
}
namespace Studio
{
	class TimelinePanel : public ITimelinePanel
	{
		Q_OBJECT

	public:
		explicit TimelinePanel(QWidget* parent = 0);
		~TimelinePanel();

		void setTimePerFrame(unsigned int msPerFrame) Q_DECL_OVERRIDE;
		uint getTimePerFrame() Q_DECL_OVERRIDE;
		void setRulerUsingFloat(bool b) Q_DECL_OVERRIDE;
		void setLabelText(const QString& text) Q_DECL_OVERRIDE; 
		void setItemCheckable(bool isCheckable) Q_DECL_OVERRIDE; 
		void addItem(const LineItem& item, const QString& icon) Q_DECL_OVERRIDE;
		LineItem item(int index) Q_DECL_OVERRIDE; 
		void setEventHandler(ITimelinePanelEvent* eventHandler) Q_DECL_OVERRIDE;
		void clear() Q_DECL_OVERRIDE;
		void UpdateVertTimeFrame(int time) Q_DECL_OVERRIDE;
		void UpdateUIKeyFramePos(int layerindex, int frameIndex, int offset)Q_DECL_OVERRIDE;
		EditPanel* getEditPanel() const Q_DECL_OVERRIDE; 
		int getRulerOffset() const Q_DECL_OVERRIDE; 
	signals:
		void frameClicked(int time);

	public slots:
		void on_playButton_clicked();
		void on_pauseButton_clicked();
		void on_resumeButton_clicked();
		void on_stopButton_clicked();
		void on_stepButton_clicked();
		void on_listWidget_itemClicked(QListWidgetItem * item);
		void on_listWidget_verticalScroll(int val);
		void on_listWidget_itemChanged(QListWidgetItem* item);
		void on_listWidget_customContextMenuRequested(const QPoint& p);
		void on_editPanelScrollArea_verticalScroll(int value);

		void onEventButtonMoved(int row, int index, float tick);
		void onEventButtonClicked(int row, int index, int subindex);
		void onFrameMoveto(float tick);
		void onCustomContextMenuRequested(int row);
		void onCustomContextMenuRequested(int row, int column);

	private:
		void onButtonClicked(ITimelinePanelEvent::Button button);

	private:
		Ui::TimelinePanel* m_ui;
		QScopedPointer<ITimelinePanelEvent> m_eventHandler;
		bool m_isCheckable; 
	};
} // namespace Studio
