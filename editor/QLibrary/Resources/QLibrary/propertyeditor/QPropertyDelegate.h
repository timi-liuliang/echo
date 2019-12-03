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

	/**
	 * PropertyDelegate 2010-03-15 Captain
	 */
	class QPropertyDelegate : public QStyledItemDelegate
	{
		Q_OBJECT

	public:
		QPropertyDelegate(QPropertyModel* model, QObject *parent = 0);

		// size
		virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		// paint
		virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		// create editor
		virtual QWidget* createEditor( QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

		// set data
		virtual void  setEditorData( QWidget* editor, const QModelIndex& index) const override;

		// set model data
		virtual void  setModelData( QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

		// update geometry
		virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& index ) const override;

	public:
		// register widget
		void registerWidget(const Echo::String& type, bool isSupportCustomPaint, QPropertyWidgetInfo::ItemDelegatePaintFun paintFun);

	protected:
		bool eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE{ return event->type() == QEvent::FocusOut; }

	private:
		// is support custom paint
		bool IsSupportCustomPaint( const Echo::String& widgetType, const QVariant& value) const;

		// item paint
		bool ItemDelegatePaint(  QPainter *painter, const Echo::String& widgetType, const QRect& rect, const QVariant& val) const;
	
	private slots:
		void commitEditor();
	
	private:
		QPropertyModel*			m_model;		// model
		QPropertyWidgetInfoMap	m_widgetInfos;
	};
}
