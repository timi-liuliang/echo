#pragma once

#include <QWidget>

namespace Studio
{
	class BackgroundWidget : public QWidget
	{
		friend class LineWidget;
		Q_OBJECT
	public:
		explicit BackgroundWidget(QWidget* parent = 0);
		~BackgroundWidget();
		void setPosition(int x);
		void setWidth(int width);
		void setText(const QString& text);
		void setIndex(int index);
		void UpdateUIKeyFramePos(int offset); 

	protected:
		virtual void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
		virtual void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

	signals:
		void widgetClicked(QString text);
		void customContextMenuRequested(int row, int column);

	private slots:
		void onCustomContextMenuRequested(const QPoint& p);

	private:
		int m_x;
		int m_width;
		int m_index;
		QString m_text;
	};

}  // namespace Studio

