#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "ResChooseDialog.h"
#include <engine/modules/gltf/gltf_assert.h>

namespace Studio
{
	static NodeTreePanel* g_inst = nullptr;

	NodeTreePanel::NodeTreePanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_newNodeDialog(nullptr)
		, m_nodeTreeMenu(nullptr)
	{
		EchoAssert(!g_inst);
		g_inst = this;

		setupUi( this);

		QObject::connect(m_newNodeButton,  SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionAddNode,  SIGNAL(triggered()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_actionImportGltfScene, SIGNAL(triggered()), this, SLOT(importGltfScene()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showSelectedNodeProperty()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(showSelectedNodeProperty()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(onChangedNodeName(QTreeWidgetItem*)));
		QObject::connect(m_nodeTreeWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));

		QObject::connect(m_actionDeleteNode, SIGNAL(triggered()), this, SLOT(onDeleteNodes()));
		QObject::connect(m_actionRenameNode, SIGNAL(triggered()), this, SLOT(onRenameNode()));
	}

	NodeTreePanel::~NodeTreePanel()
	{

	}

	NodeTreePanel* NodeTreePanel::instance()
	{
		return g_inst;
	}

	void NodeTreePanel::showNewNodeDialog()
	{
		if (!m_newNodeDialog)
			m_newNodeDialog = new NewNodeDialog(this);

		m_newNodeDialog->setVisible(true);
	}


	void NodeTreePanel::refreshNodeTreeDisplay()
	{
		m_nodeTreeWidget->clear();

		// begin with "Node"
		addNode(EchoEngine::instance()->getCurrentEditNode(), m_nodeTreeWidget->invisibleRootItem(), true);

		m_nodeTreeWidget->expandAll();
	}

	void NodeTreePanel::addNode(Echo::Node* node, QTreeWidgetItem* parent, bool recursive)
	{
		Echo::String nodeName = node->getName();
		Echo::String nodeClassName = node->getClassName();

		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeClassName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		Echo::String iconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(parent);
		nodeItem->setText(0, nodeName.c_str());
		nodeItem->setIcon(0, QIcon(iconPath.c_str()));
		nodeItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)node));
		nodeItem->setFlags( nodeItem->flags() | Qt::ItemIsEditable);

		// show property
		m_nodeTreeWidget->setCurrentItem(nodeItem);

		if (recursive)
		{
			for (Echo::ui32 i = 0; i < node->getChildNum(); i++)
			{
				Echo::Node* childNode = node->getChild(i);
				addNode(childNode, nodeItem, recursive);
			}
		}
	}

	Echo::Node* NodeTreePanel::getCurrentSelectNode()
	{
		if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
			return nullptr;

		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem() ? m_nodeTreeWidget->currentItem() : m_nodeTreeWidget->invisibleRootItem()->child(0);
		if (item)
		{
			return (Echo::Node*)item->data(0, Qt::UserRole).value<void*>();
		}

		return nullptr;
	}

	void NodeTreePanel::addNode(Echo::Node* node)
	{
		if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
		{
			EchoEngine::instance()->setCurrentEditNode(node);
			refreshNodeTreeDisplay();
		}
		else
		{
			QTreeWidgetItem* parentItem = m_nodeTreeWidget->currentItem() ? m_nodeTreeWidget->currentItem() : m_nodeTreeWidget->invisibleRootItem()->child(0);
			Echo::Node* parentNode = (Echo::Node*)parentItem->data(0, Qt::UserRole).value<void*>();
			if (parentNode)
			{
				node->setParent(parentNode);

				addNode(node, parentItem, false);

				parentItem->setExpanded(true);
			}
		}

		// show property
		showSelectedNodeProperty();
	}

	// node tree widget show menu
	void NodeTreePanel::showMenu(const QPoint& point)
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->itemAt(point);
		if (item)
		{
			m_nodeTreeWidget->setCurrentItem(item);

			EchoSafeDelete(m_nodeTreeMenu, QMenu);
			m_nodeTreeMenu = EchoNew(QMenu);
			m_nodeTreeMenu->addAction(m_actionAddNode);
			m_nodeTreeMenu->addAction(m_actionAddChildScene);
			m_nodeTreeMenu->addAction(m_actionImportGltfScene);
			m_nodeTreeMenu->addSeparator();
			m_nodeTreeMenu->addAction(m_actionRenameNode);
			m_nodeTreeMenu->addSeparator();
			m_nodeTreeMenu->addAction(m_actionDeleteNode);
			m_nodeTreeMenu->exec(QCursor::pos());
		}
		else
		{
			EchoSafeDelete(m_nodeTreeMenu, QMenu);
			m_nodeTreeMenu = EchoNew(QMenu);
			m_nodeTreeMenu->addAction(m_actionAddNode);
			m_nodeTreeMenu->addAction(m_actionAddChildScene);
			m_nodeTreeMenu->addAction(m_actionImportGltfScene);
			m_nodeTreeMenu->exec(QCursor::pos());
		}
	}

	// on trigger delete nodes
	void NodeTreePanel::onDeleteNodes()
	{
		QList<QTreeWidgetItem*> items = m_nodeTreeWidget->selectedItems();
		while (items.size() > 0)
		{
			QTreeWidgetItem* item = items[0];

			Echo::Node* node = (Echo::Node*)item->data(0, Qt::UserRole).value<void*>();

			// remove item from ui
			QTreeWidgetItem* parentItem = item->parent();
			if (parentItem)
				parentItem->removeChild(item);
			else
				m_nodeTreeWidget->invisibleRootItem()->removeChild(item);
			
			// remove item from node tree
			node->remove();
			node->queueFree();

			items = m_nodeTreeWidget->selectedItems();
		}

		// set as nullptr
		if (m_nodeTreeWidget->invisibleRootItem()->childCount() == 0)
		{
			EchoEngine::instance()->setCurrentEditNode( nullptr);
		}

		// update property panel display
		showSelectedNodeProperty();
	}

	// on trigger rename node
	void NodeTreePanel::onRenameNode()
	{
		QTreeWidgetItem* item = m_nodeTreeWidget->currentItem();
		if (item)
		{
			m_nodeTreeWidget->editItem( item);
		}
	}

	// when modifyd item name
	void NodeTreePanel::onChangedNodeName(QTreeWidgetItem* item)
	{
		if (item)
		{
			Echo::Node* node = (Echo::Node*)item->data(0, Qt::UserRole).value<void*>();
			if (!node)
				return;

			Echo::String newName = item->text(0).toStdString().c_str();
			if (newName.empty())
				item->setText(0, node->getName().c_str());

			Echo::Node* parent = node->getParent();
			if (parent)
			{
				if (parent->isChildExist(newName))
				{
					item->setText(0, node->getName().c_str());
				}
				else
				{
					node->setName(newName);
				}
			}
			else
			{
				node->setName(newName);
			}
		}
	}

	// import gltf scene
	void NodeTreePanel::importGltfScene()
	{
		Echo::String gltfFile = ResChooseDialog::getExistingFile(this, ".gltf");
		if (!gltfFile.empty())
		{
			Echo::GltfResPtr asset = Echo::GltfRes::create( gltfFile);
			Echo::Node* node = asset->build();
			if (node)
			{
				addNode(node);
			}
		}
	}

	// 显示当前选中节点属性
	void NodeTreePanel::showSelectedNodeProperty()
	{
		m_propertyHelper.clear();
		m_propertyHelper.setHeader("Property", "Value");

		Echo::Node* node = getCurrentSelectNode();
		if (node)
		{
			showNodePropertyRecursive(node, node->getClassName());
			m_propertyHelper.applyTo(node->getName(), m_propertyTreeView, this, SLOT(refreshPropertyToNode(const QString&, QVariant)), false);
		}
		else
		{
			m_propertyHelper.applyTo("empty", m_propertyTreeView, this, SLOT(refreshPropertyToNode(const QString&, QVariant)), false);
		}	
	}

	// 显示属性
	void NodeTreePanel::showNodePropertyRecursive(Echo::Object* classPtr, const Echo::String& className)
	{
		// show parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if(parentClassName!="Object")
				showNodePropertyRecursive(classPtr, parentClassName);
		}

		// show self property
		m_propertyHelper.beginMenu(className.c_str());
		{
			const Echo::PropertyInfos& propertys = Echo::Class::getPropertys(className);
			for (const Echo::PropertyInfo& prop : propertys)
			{
				Echo::Variant var;
				Echo::Class::getPropertyValue( classPtr, prop.m_name, var);

				showPropertyByVariant( prop.m_name, var);
			}
		}
		m_propertyHelper.endMenu();
	}

	// show property
	void NodeTreePanel::showPropertyByVariant(const Echo::String& name, const Echo::Variant& var)
	{
		switch (var.getType())
		{
		case Echo::Variant::Type_String:		m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_None); break;
		case Echo::Variant::Type_Vector3:		m_propertyHelper.addItem(name.c_str(), var.toVector3(), QT_UI::WT_Vector3); break;
		case Echo::Variant::Type_ResourcePath:	m_propertyHelper.addItem(name.c_str(), var.toResPath().getPath(), QT_UI::WT_AssetsSelect, var.toResPath().getSupportExts().c_str());break;
		default:								m_propertyHelper.addItem(name.c_str(), var.toString(), QT_UI::WT_None); break;
		}
		
	}

	// 属性修改后,更新结点值
	void NodeTreePanel::refreshPropertyToNode(const QString& property, QVariant value)
	{
		Echo::String propertyName = property.toStdString().c_str();
		Echo::String valStr = value.toString().toStdString().c_str();
		Echo::Node* node = getCurrentSelectNode();
		Echo::Variant::Type type = Echo::Class::getPropertyType(node, propertyName);

		Echo::Variant propertyValue;
		if(propertyValue.fromString(type, valStr))
		{
			Echo::Class::setPropertyValue(node, propertyName, propertyValue);
		}
		else
		{
			EchoLogError("Can't set property [%s] value [%s]", propertyName.c_str(), valStr.c_str());
		}
	}
}