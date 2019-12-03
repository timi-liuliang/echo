#pragma once

#include <QHBoxLayout>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "ui_ShaderEditor.h"

namespace Studio
{
	class ShaderEditor : public QDockWidget , public Ui_ShaderEditorPanel
	{
		Q_OBJECT

	public:
		ShaderEditor(QWidget* parent = 0);
		~ShaderEditor();

	private:
		QHBoxLayout*		m_horizonLayout = nullptr;
		QGraphicsScene*		m_graphicsScene = nullptr;
		QGraphicsView*		m_graphicsView = nullptr;
	};
}