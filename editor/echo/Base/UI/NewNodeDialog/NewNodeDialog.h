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

		// recover edit settings
		void recoverEditSettings();

	private:
		NewNodeDialog(QWidget* parent = 0);
		~NewNodeDialog();

		void initNodeDisplayByModule();
		void addNode(const Echo::String& nodeName);

		void initNodeDisplayByInherite();
		void addNode(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private:
		// get module item by nodeName
		QTreeWidgetItem* getModuleItem(const Echo::String& nodeName);

		// create QTreewidgetItem by nodename
		QTreeWidgetItem* createQTreeWidgetItemByNodeName(const Echo::String& nodeName, QTreeWidgetItem* parent);

	private slots:
		void onSelectNode();
		void onConfirmNode();

		// on switch node view type
		void onSwitchNodeVeiwType();

	private:
		Echo::String	m_result;
		bool			m_viewNodeByModule;
	};
}