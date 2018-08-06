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

		// 获取面板实例
		static NodeTreePanel* instance();

		// 保存当前编辑资源
		void saveCurrentEditRes();

	public:
		// clear
		void clear();

		// 刷新结点树显示
		void refreshNodeTreeDisplay();

		// 添加结点
		void addNode(Echo::Node* node);

		// 添加结点
		void addNode( Echo::Node* node, QTreeWidgetItem* parent, bool recursive);

		// 获取当前结点
		Echo::Node* getCurrentSelectNode();

		// 获取当前编辑对象
		Echo::Object* getCurrentEditObject();

		// 设置编辑节点
		void setNextEditObject(Echo::Object* obj) { m_nextEditObject = obj; }

	private slots:
		// 显示新建节点窗口
		void showNewNodeDialog();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// 显示当前选中节点属性
		void showSelectedObjectProperty();

		// 属性修改后,更新结点值
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

	public slots:
		// 渲染
		void  update();

		// on select node
		void onSelectNode();

		// on clicked node item
		void onClickedNodeItem(QTreeWidgetItem* item, int column);

		// edit res
		void onEditObject(Echo::Object* res);

		// edit res
		void onSelectRes(const Echo::String& resPath);

	protected:
		// update item display
		void updateNodeTreeWidgetItemDisplay(QTreeWidgetItem* item);

	private:
		// 递归显示属性
		void showObjectPropertyRecursive(Echo::Object* classPtr, const Echo::String& className);

		// show property
		void showPropertyByVariant(const Echo::String& name, const class Echo::Variant& var, const Echo::PropertyInfo* propInfo);

	private:
		// remove item
		void removeItem(QTreeWidgetItem* item);

	private:
		int								m_width;
		QTimer*							m_timer;				// 计时器
		QT_UI::QPropertyConfigHelper	m_propertyHelper;		// 属性
		Echo::Object*					m_nextEditObject;		
		Echo::Object*					m_currentEditObject;
		QMenu*							m_nodeTreeMenu;			// 结点树右键菜单
	};
}