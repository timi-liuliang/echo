#pragma once

#include <QWidget>
#include <QDockWidget>

namespace Studio
{
	/**
	 * TimelinePanel½Ó¿Ú
	 */
	class ITimelinePanelEvent;
	class LineItem;
	class EditPanel; 
	class ITimelinePanel : public QDockWidget
	{
	public:
		ITimelinePanel(QWidget* parent) : QDockWidget(parent) {}
		virtual ~ITimelinePanel() {}

		virtual void setTimePerFrame(unsigned int msPerFrame) = 0;
		virtual uint getTimePerFrame() = 0;
		virtual void setRulerUsingFloat(bool b) = 0; 
		virtual void setLabelText(const QString& text) = 0; 
		virtual void setItemCheckable(bool checkable) = 0; 
		virtual void addItem(const LineItem& item, const QString& icon)  = 0;
		virtual LineItem item(int index) = 0; 
		virtual void UpdateUIKeyFramePos(int layerindex, int frameIndex, int offset) = 0;
		virtual void setEventHandler(ITimelinePanelEvent* eventHandler) = 0;
		virtual void clear() = 0;
		virtual void UpdateVertTimeFrame(int time) = 0;
		virtual EditPanel* getEditPanel() const = 0; 
		virtual int getRulerOffset() const = 0;
	};

}
