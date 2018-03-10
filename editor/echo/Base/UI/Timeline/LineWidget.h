#pragma once

#include <QWidget>

namespace Studio
{
	class SlideButton;
	class BackgroundWidget; 
	class LineWidget : public QWidget
	{
		Q_OBJECT
	public:
		explicit LineWidget(QString name, QWidget* parent = 0);
		~LineWidget();

		template <typename Widget>
		void addSubWidget(Widget* w)
		{ 
			w->move(w->m_x, 0); 
		}

		SlideButton* getButton(int pos); 

		SlideButton* getButtonByIndex(int idx); 

		int getButtonCounts() const { return m_items.size(); }

		QString getName() const { return m_name; }
		//ªÊ÷∆±≥æ∞¥Û–°
		QRectF getRealRect(int yindex);

		BackgroundWidget* getChainedWidget(SlideButton* button); 

	public: 
		bool onBackgroundWidgetClicked(QString text); 
	signals:
		void customContextMenuRequested(int row, int column);

	public:
		QString						 m_name; 
		QList<SlideButton*>			 m_items;
		QList<BackgroundWidget*>	 m_anims; 

	private slots:
		void onCustomContextMenuRequested(const QPoint& pos);

	};

}  // namespace Studio
