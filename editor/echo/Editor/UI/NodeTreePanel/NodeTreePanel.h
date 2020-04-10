#pragma once

#include <QDockWidget>
#include "ui_NodeTreePanel.h"
#include "QProperty.hpp"
#include "NewNodeDialog.h"
#include <engine/core/resource/Res.h>
#include <engine/core/scene/node.h>

namespace Studio
{
	class NodeTreePanel : public QDockWidget, public Ui_NodeTreePanel
	{
		Q_OBJECT

	public:
		NodeTreePanel( QWidget* parent=0);
		~NodeTreePanel();

		// instance
		static NodeTreePanel* instance();

		// save current edit resource
		void saveCurrentEditRes();

	public:
		// clear
		void clear();

		// refresh node tree display
		void refreshNodeTreeDisplay();

		// add node
		void addNode(Echo::Node* node);

		// get current select node
		Echo::Node* getCurrentSelectNode();

		// get current edit object
		Echo::Object* getCurrentEditObject();

		// set next edit object
		void setNextEditObject(Echo::Object* obj) { m_nextEditObject = obj; }

	public:
		// refresh node tree display
		static void refreshNodeTreeDisplay(QTreeWidget* treeWidget);

		// add node
		static void addNode(QTreeWidget* treeWidget, Echo::Node* node, QTreeWidgetItem* parent, bool recursive);

		// get node in the item
		static Echo::Node* getNode(QTreeWidgetItem* item);

		// get node icon
		static Echo::String getNodeIcon(Echo::Object* node);

	private slots:
		// show new node dialog
		void showNewNodeDialog();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// show selected object property
		void showSelectedObjectProperty();

		// refresh object property display
		void refreshPropertyToObject(const QString& property, QVariant value);

		// on trigger delete nodes
		void onDeleteNodes();

		// on duplicate node
		void onDuplicateNode();

		// on change type
		void onChangeType();

		// on trigger rename node
		void onRenameNode();

		// on save branch as scene
		void onSaveBranchAsScene();

		// on discard instancing
		void onDiscardInstancing();

		// when modifyd item name
		void onChangedNodeName(QTreeWidgetItem* item);

		// import gltf scene
		void importGltfScene();

		// on instance child scene
		void onInstanceChildScene();

		// refresh node display
		void refreshNodeDisplay(QTreeWidgetItem* item);

		// node tree drag drop operator
		void onItemPositionChanged(QTreeWidgetItem* item);
        
    public slots:
        // property tree menu
        void showPropertyMenu(const QPoint& point);

		// on reference other property
		void onReferenceProperty();

		// on delete reference
		void onDeletePropertyReference();

		// reset to default
		void onPropertyResetToDefault();
        
    private slots:
        // signal tree widget show menu
        void showSignalTreeWidgetMenu(const QPoint& point);
        
        // connect Object slot
        void onConnectOjectSlot();

		// on disconnect all
		void onSignalDisconnectAll();
		void onSignalDisconnect();
		void onSignalGotoMethod();
        
    public slots:
        // update signals display
        void showSelectedObjectSignal();
        
        // update signal display recursively
        void showObjectSignalRecursive(Echo::Object* classPtr, const Echo::String& className);

	public slots:
		// update
		void  update();

		// on select node
        void onUnselectCurrentEditObject();
		void onSelectNode();

		// on clicked node item
		void onClickedNodeItem(QTreeWidgetItem* item, int column);
		void onDoubleClickedNodeItem(QTreeWidgetItem* item, int column);

		// edit res
		void onEditObject(Echo::Object* res);

		// edit res
		void onSelectRes(const Echo::String& resPath);

	protected:
		// update item display
		static void updateNodeTreeWidgetItemDisplay(QTreeWidget* treeWidget, QTreeWidgetItem* item);

	private:
		// show object property recursively
		void showObjectPropertyRecursive(Echo::Object* classPtr, const Echo::String& className);

		// show property
		void showPropertyByVariant(Echo::Object* object, const Echo::String& name, const class Echo::Variant& var, const Echo::PropertyInfo* propInfo);

	private:
		// remove item
		void removeItem(QTreeWidgetItem* item);
        
        // update signal tab visible
        void updateSignaltabVisible();

	private:
		int								m_width;
		QTimer*							m_timer;
		QT_UI::QPropertyConfigHelper	m_propertyHelper;
		Echo::Object*					m_nextEditObject = nullptr;
		Echo::Object*					m_currentEditObject = nullptr;
		QMenu*							m_nodeTreeMenu = nullptr;
        QMenu*                          m_propertyMenu = nullptr;
        QMenu*                          m_signalTreeMenu = nullptr;
        Echo::String                    m_signalName;
		Echo::String					m_connectStr;
        Echo::String                    m_propertyTarget;
	};
}
