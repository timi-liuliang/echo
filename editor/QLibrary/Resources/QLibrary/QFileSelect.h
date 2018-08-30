#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

namespace QT_UI
{
	//-------------------------------------------
	// FileSelect 文件选择控件 2013-1-22 帝林
	//-------------------------------------------
	class QFileSelect : public QWidget
	{
		Q_OBJECT
	public:
		QFileSelect( QWidget* parent = 0);

		// 设置路径
		void SetPath( QString text) { m_lineEdit->setText( text);  }

		// 获取路径
		QString GetPath() { return m_lineEdit->text(); }

	private slots:
		// 选择路径
		void OnSelectPath();

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
	};
}