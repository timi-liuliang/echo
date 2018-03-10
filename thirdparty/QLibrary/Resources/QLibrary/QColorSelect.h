#pragma once

#include <Engine/Core.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

using namespace std;

namespace QT_UI
{
	//-------------------------------------------
	// ColorSelect 颜色选择按钮 2013-3-4 帝林
	//-------------------------------------------
	class QColorSelect : public QPushButton
	{
		Q_OBJECT
	public:
		QColorSelect( QWidget* parent = 0);

		// 设置颜色
		void SetColor( string color);

		// 设置颜色
		void SetColor( const Echo::Color& color);

		// 获取颜色
		string GetColor();

		// 获取颜色
		const Echo::Color& GetColor() const { return m_color; }

		// MVC渲染
		static void ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);

	protected:
		// 重定义渲染
		void paintEvent( QPaintEvent* event); 

		// 大小
		QSize sizeHint() const;

	public slots:
		// 选择颜色
		void OnSelectColor();

	private:
		Echo::Color		m_color;
	};
}