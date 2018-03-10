#pragma once

#include <QDockWidget>
#include "ui_ResPanel.h"
#include "QProperty.hpp"
#include <QDirectoryModel.h>

namespace Studio
{
	class ResPanel : public QDockWidget, public Ui_ResPanel
	{
	public:
		ResPanel( QWidget* parent=0);
		~ResPanel();

	private:
		QT_UI::QDirectoryModel*		m_dirModel;		// Ŀ¼
	};
}