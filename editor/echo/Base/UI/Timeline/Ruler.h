#pragma once

#include <QWidget>

namespace Studio
{
	class Ruler : public QWidget
	{
		Q_OBJECT

	public:
		explicit Ruler(QWidget* parent = 0);
		//更新面板的时间纵轴
		void SetTimePerFrame(unsigned int msPerFrame);
		uint getTimePerFrame();
		void UpdateVertTimeFrame(int time);
		void mousePressEvent(QMouseEvent *e);
		void setUsingFloat(bool usingFloat) { m_isUsingFloat = usingFloat; }
		int getRulerOffset() const { return offsetValue; }

	public slots:
		void on_editPanelScrollArea_horizontalScroll(int value);

	signals:
		void frameClicked(int);

	protected:
		// Ruler painer code
		void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;

	private:
		bool m_isUsingFloat; 
		int m_CurrentFrame;
		int offsetValue;
		unsigned int m_msPerFrame;
		unsigned int m_framePerSecond; 
	};
}

