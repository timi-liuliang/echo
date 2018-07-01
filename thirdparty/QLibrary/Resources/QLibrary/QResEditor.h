#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QPainter>
#include <functional>
#include "engine/core/util/StringUtil.h"

namespace QT_UI
{
	class QResEdit : public QWidget
	{
		Q_OBJECT

	public:
		QResEdit(class QPropertyModel* model, QString propertyName, const char* resType, QWidget* parent);

		// 设置路径
		void SetId( QString text) { m_lineEdit->setText( text);  }

		// 获取路径
		QString GetId() { return m_lineEdit->text(); }

		// MVC渲染
		static void ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val);

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

		// is texture res
		bool isTextureRes();

		// correct size
		void adjustHeightSize();

	private slots:
		// 选择路径
		void OnSelectPath();

		// edit finished
		void onEditFinished();

	private:
		Echo::String	m_id;
		Echo::String	m_resType;
		Echo::String	m_exts;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}