#pragma once

#include <QPushButton>
#include <QPoint>
#include <QList>

namespace Studio
{
	class EditPanel;
	class SlideButton : public QPushButton
	{
		friend class LineWidget;
		Q_OBJECT

	public: 
		struct SubData
		{
			QString name; 
			QColor color; 

			SubData(const QString& _name, const QColor& _color) : name(_name), color(_color){}
		};

	public:
		explicit SlideButton(QWidget* parent = 0);
		~SlideButton();
		void setPosition(int x);
		int getPosition() const; 
		int getFrame() const; 
		void setWidth(int width);
		void setIndex(const QPoint& p);
		void setColor(QColor color);
		void setIsLastKey(bool flag);
		void UpdateUIKeyFramePos(int offset);
		void setParentPanel(EditPanel* panel);
		void setSelected(bool flag);
		
		void append(const SubData& sub); 
		const QList<SubData>& getSubDatas() const; 

		void onManualClicked(int subIndex); 
		int contain(QString text); 

		QString getName(int subindex) const; 
		int getNameSize() const { return m_subDatas.size() + 1; }

	protected:
		virtual void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
		virtual void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
		virtual void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
		//virtual void mouseDoubleClickEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
		virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

	signals:
		void keyClicked(int row, int index, int subIndex);
		void moved(int row, int index, float tick);
		void customContextMenuRequested(int row, int column);

	private slots:
		void onCustomContextMenuRequested(const QPoint& p);

	private:
		int m_x;
		int m_width;
		QPoint m_index;
		QPoint m_mousePressPos;
		QPoint m_mouseMovePos;
		bool m_bIsLastKey;
		EditPanel* m_parentPanel;
		bool m_selected;
		QColor m_color;

		QList<SubData> m_subDatas; 
	};

}  // namespace Studio
