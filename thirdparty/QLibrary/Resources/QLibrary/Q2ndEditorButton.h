#pragma once

#include <Engine/Core.h>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

using namespace std;

namespace QT_UI
{
	class QPropertyModel; 

	class Q2ndEditor : public QPushButton
	{
		Q_OBJECT

	public:
		Q2ndEditor(QPropertyModel* model, QWidget* parent = 0);
		~Q2ndEditor(); 

	public slots:
		// 
		void onClicked();

	private:
		QPropertyModel* m_model; 
	};
}