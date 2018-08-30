#pragma once

#include <QtGui/QPixmap>
#include <QRadioButton>

namespace QT_UI
{
	//----------------------------------------
	// 新建场景    2011-01-11
	//----------------------------------------
	class QRadioImage : public QRadioButton
	{
		Q_OBJECT

	public:
		// 构造函数
		QRadioImage( QWidget* parent = 0);

	public:
		// 设置显示图片
		bool  SetImage( const char* resPath);

	signals:
		// 图象改变
		void imageChanged( QString resPath);

	private slots:
		// 重载
		void  paintEvent(QPaintEvent * e);

		// 鼠标双击事件
		void  mouseDoubleClickEvent ( QMouseEvent * event );

	private:
		QString       m_resPath;
		QPixmap       m_image;
	};
}