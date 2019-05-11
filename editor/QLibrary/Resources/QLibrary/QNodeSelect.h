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
	class QNodeSelect : public QWidget
	{
		Q_OBJECT

	public:
		QNodeSelect(class QPropertyModel* model, QString propertyName, const char* exts, const char* files, QWidget* parent = 0);

		// 设置路径
		void SetPath( const Echo::String text) { m_lineEdit->setText( text.c_str());  }

		// 获取路径
		QString GetPath() { return m_lineEdit->text(); }

		// MVC渲染
		//static bool ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val);

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
		Echo::String	m_exts;
		QString			m_files;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}