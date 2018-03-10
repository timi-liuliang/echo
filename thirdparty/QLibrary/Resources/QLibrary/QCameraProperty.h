#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <functional>

namespace QT_UI
{
	//-------------------------------------------
	// 获取主摄像机位置 
	//-------------------------------------------
	class QCameraPosDir : public QWidget
	{
		Q_OBJECT
	public:
		QCameraPosDir(QWidget* parent = 0);

		// 设置路径
		void SetPos( QString text) { m_lineEdit->setText( text);  }

		// 获取路径
		QString GetPos() { return m_lineEdit->text(); }

	private slots:
		// 选择路径
		void OnCatchData();

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
	};
}