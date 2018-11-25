#pragma once

#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QEvent>
#include "QPropertyModel.h"
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	struct QPropertyWidgetInfo
	{
		typedef bool(*ItemDelegatePaintFun)(QPainter*, const QRect&, const string&);

		Echo::String		 m_type;
		bool				 m_isSupportCustomPaint;
		ItemDelegatePaintFun m_paintFun;
	};
	typedef map<Echo::String, QPropertyWidgetInfo>	QPropertyWidgetInfoMap;

	//------------------------------------------
	// 属性专用(用途有限) 2010-03-15  帝林
	//------------------------------------------
	class QPropertyDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

	public:
		// 构造函数
		QPropertyDelegate(QPropertyModel* model, QObject *parent = 0);

		// size
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		// 绘制函数
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		// 创建编辑器控件
		virtual QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		// 设置控件数据
		virtual void  setEditorData( QWidget* editor, const QModelIndex& index) const override;

		// 设置模型数据
		virtual void  setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		// 设置编辑器Geometry
		virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index ) const override;

	public:
		// register widget
		void registerWidget(const Echo::String& type, bool isSupportCustomPaint, QPropertyWidgetInfo::ItemDelegatePaintFun paintFun);

	protected:
		bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE{ return event->type() == QEvent::FocusOut; }

	private:
		// 是否拥有自定义渲染
		bool IsSupportCustomPaint( const Echo::String& widgetType, const QVariant& value) const;

		// 自定义渲染
		bool ItemDelegatePaint(  QPainter *painter, const Echo::String& widgetType, const QRect& rect, const QVariant& val) const;
	
	private slots:
		void commitEditor();
	
	private:
		QPropertyModel*			m_model;		// model
		QPropertyWidgetInfoMap	m_widgetInfos;
	};
}
