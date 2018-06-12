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

	public:
		// 刷新结点树显示
		void refreshNodeTreeDisplay();

		// 添加结点
		void addNode(Echo::Node* node);

		// 添加结点
		void addNode( Echo::Node* node, QTreeWidgetItem* parent, bool recursive);

		// 获取当前结点
		Echo::Node* getCurrentSelectNode();

	private slots:
		// 显示新建节点窗口
		void showNewNodeDialog();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// 显示当前选中节点属性
		void showSelectedNodeProperty();

		// 属性修改后,更新结点值
		void refreshPropertyToNode(const QString& property, QVariant value);

		// on trigger delete nodes
		void onDeleteNodes();

		// on trigger rename node
		void onRenameNode();

		// when modifyd item name
		void onChangedNodeName(QTreeWidgetItem* item);

		// import gltf scene
		void importGltfScene();

	public slots:
		// show res property
		void showResProperty(const Echo::String& resPath);

	private:
		// 递归显示属性
		void showNodePropertyRecursive(Echo::Object* classPtr, const Echo::String& className);

		// 递归显示资源属性
		void showResPropertyRecursive(Echo::Object* classPtr, const Echo::String& className);

		// show property
		void showPropertyByVariant(const Echo::String& name, const class Echo::Variant& var);

	private:
		NewNodeDialog*					m_newNodeDialog;		// 新建结点
		QT_UI::QPropertyConfigHelper	m_propertyHelper;		// 属性
		Echo::ResPtr					m_currentEditRes;		// 当前资源
		QMenu*							m_nodeTreeMenu;			// 结点树右键菜单
	};
}