#pragma once

#include <engine/core/Util/StringUtil.h>
#include <QDialog>
#include "QMenubarEx.h"
#include "ui_NewNodeDialog.h"

namespace Studio
{
	class NewNodeDialog : public QDialog, public Ui_NewNodeDialog
	{
		Q_OBJECT

	public:
		NewNodeDialog(QWidget* parent = 0);
		~NewNodeDialog();

	private:
		void initNodeDisplay();
		void addNode(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private slots:
		void onConfirmNode();
	};
}