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
		// instance
		static NewNodeDialog* instance();

		// get node type
		static Echo::String getSelectedNodeType();

	private:
		NewNodeDialog(QWidget* parent = 0);
		~NewNodeDialog();

		void initNodeDisplay();
		void addNode(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private slots:
		void onSelectNode();
		void onConfirmNode();

	private:
		Echo::String	m_result;
	};
}