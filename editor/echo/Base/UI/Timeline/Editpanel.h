#pragma once

#include <QWidget>
#include <QList>
#include "LineItem.h"

namespace Studio
{
	class LineWidget;
	class EditPanel : public QWidget
	{
		Q_OBJECT
	public:
		EditPanel(QWidget* parent = 0);

		void addItem(const LineItem& item);
		void clear();
		LineWidget* getLine(int row); 
		void SetTimePerFrame(unsigned int msPerFrame);
		void UpdateVertTimeFrame(int time);
		void UpdateUIKeyFramePos(int layerindex, int frameIndex, int offset);
		void setSelectedKeyFrame(int row,int index);
		void setSelectedLayer(int row);

	protected:
		virtual void EditPanel::paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

	private: 
		int  getPixelWidth(float time) const; 

		void addDurationWidget(LineWidget* lineWidget, const LineNode& node);

		void addSlideButton(LineWidget* background, const LineNode& node, int position, int& buttonIdx);

	private:
		QObject* getTimelinePanel();

	private slots:
		void onBackgroundWidgetClicked(QString text); 

	private:
		QList<LineWidget*> m_items;
		QObject* m_timelinePanel;
		unsigned int m_msPerFrame; 
		int m_CurrentFrame;
		int m_CurSelectedKeyRowIndex;
		int m_CurSelectedKeyColumnIndex;
		int m_CurSelectedLayerIndex;
	};

}  // namespace Studio
