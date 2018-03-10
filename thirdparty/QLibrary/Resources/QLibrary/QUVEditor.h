#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

class QPropertyModel;
namespace QT_UI
{
	class UVChoosePanel;
	class QUVEditor : public QWidget
	{
		Q_OBJECT
	public:
		QUVEditor(QPropertyModel* model,QString propertyName,QWidget* parent = 0);

		// 设置向量
		void SetUV(QString text) { m_lineEdit->setText(text); }

		// 获取向量
		QString GetUV() { return m_lineEdit->text(); }

		private slots:

		void OnEditUV();
		void UpdateUV();

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		QPropertyModel* propertyModel;
		QString			propertyName;
	};
}