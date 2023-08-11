#pragma once

#include <engine/core/util/StringUtil.h>
#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenuBarEx.h>
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
		void addNode(const Echo::String& nodeName, QStandardItem* parent);

	private:
		// get module item by nodeName
		QStandardItem* getModuleItem(const Echo::String& nodeName);

		// create QTreewidgetItem by nodename
		QStandardItem* createQTreeWidgetItemByNodeName(const Echo::String& nodeName, QStandardItem* parent, bool isCreateWhenNodeIsVirtual);

	private slots:
		void onSelectNode(QModelIndex index);
		void onConfirmNode();

		// on switch node view type
		void onSwitchNodeVeiwType();

		// search node
		void onSearchTextChanged();

	private:
		Echo::String			m_result;
		bool					m_viewNodeByModule;
		QStandardItemModel*		m_standardModel;
		QSortFilterProxyModel*	m_filterProxyModel;
	};
}
