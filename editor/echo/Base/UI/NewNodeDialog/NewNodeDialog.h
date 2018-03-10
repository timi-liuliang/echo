#pragma once

#include <engine/core/Util/StringUtil.h>
#include <QMainWindow>
#include "ui_NewNodeDialog.h"

namespace Studio
{
	class NewNodeDialog : public QMainWindow, public Ui_NewNodeDialog
	{
	public:
		NewNodeDialog(QWidget* parent = 0);
		~NewNodeDialog();

	private:
		void initNodeDisplay();

		void addNode(const Echo::String& nodeName, QTreeWidgetItem* parent);
	};
}