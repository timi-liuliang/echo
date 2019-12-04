#pragma once

#include <QHBoxLayout>
#include <QDockWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "ui_ScratchEditor.h"

namespace Studio
{
	class ScratchEditor : public QDockWidget, public Ui_ScratchEditorPanel
	{
		Q_OBJECT

	public:
		ScratchEditor(QWidget* parent = 0);
		~ScratchEditor();

	private:
	};
}