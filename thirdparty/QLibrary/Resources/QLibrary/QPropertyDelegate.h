#pragma once

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QEvent>
#include "QPropertyModel.h"

namespace QT_UI
{
	//------------------------------------------
	// 属性专用(用途有限) 2010-03-15  帝林
	//------------------------------------------
	class QPropertyDelegate : public QStyledItemDelegate
	{
		Q_OBJECT
	public:
		// 构造函数
		QPropertyDelegate(QPropertyModel* model, QObject *parent = 0);

		// 绘制函数
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

		// 创建编辑器控件
		QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

		// 设置控件数据
		void  setEditorData( QWidget* editor, const QModelIndex& index) const;

		// 设置模型数据
		void  setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

		// 设置编辑器Geometry
		void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index ) const;

	protected:
		bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE{ return event->type() == QEvent::FocusOut; }

	private:
		// 是否拥有自定义渲染
		bool IsSupportCustomPaint( const QString& widgetType) const;

		// 自定义渲染
		void ItemDelegatePaint(  QPainter *painter, const QString& widgetType, const QRect& rect, const QVariant& val) const;
	
	private slots:
		void commitEditor();
	
	private:
		QPropertyModel*   m_model;	// model
	};
}
