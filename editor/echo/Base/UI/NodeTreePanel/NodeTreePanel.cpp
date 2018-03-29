#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "NodeTreePanel.h"
#include "EchoEngine.h"

namespace Studio
{
	static NodeTreePanel* g_inst = nullptr;

	NodeTreePanel::NodeTreePanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_newNodeDialog(nullptr)
	{
		EchoAssert(!g_inst);
		g_inst = this;

		setupUi( this);

		QObject::connect(m_newNodeButton, SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
		QObject::connect(m_nodeTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(showSelectedNodeProperty()));
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
		addNode(EchoEngine::Instance()->getCurrentEditNode(), m_nodeTreeWidget->invisibleRootItem(), true);

		m_nodeTreeWidget->expandAll();
	}

	void NodeTreePanel::addNode(Echo::Node* node, QTreeWidgetItem* parent, bool recursive)
	{
		Echo::String nodeName = node->getName();

		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		Echo::String iconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(parent);
		nodeItem->setText(0, nodeName.c_str());
		nodeItem->setIcon(0, QIcon(iconPath.c_str()));
		nodeItem->setData(0, Qt::UserRole, QVariant::fromValue((void*)node));

		QObject::connect(m_nodeTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(addNodeToScene(QTreeWidgetItem *item, int)));

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
			EchoEngine::Instance()->setCurrentEditNode(node);
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
	}

	// 显示当前选中节点属性
	void NodeTreePanel::showSelectedNodeProperty()
	{
		m_propertyHelper.clear();
		m_propertyHelper.setHeader("Property", "Value");

		Echo::Node* node = getCurrentSelectNode();
		if (node)
		{
			showNodePropertyRecursive(node->getClassName());
		}

		m_propertyHelper.applyTo(node->getName(), m_propertyTreeView, this, SLOT(refreshPropertyToNode(const QString&, QVariant)), false);
	}

	// 显示属性
	void NodeTreePanel::showNodePropertyRecursive(const Echo::String& className)
	{
		// show parent property first
		Echo::String parentClassName;
		if (Echo::Class::getParentClass(parentClassName, className))
		{
			// don't display property of object
			if(parentClassName!="Object")
				showNodePropertyRecursive(parentClassName);
		}

		// show self property
		m_propertyHelper.beginMenu(className.c_str());
		{
			const Echo::PropertyInfos& propertys = Echo::Class::getPropertys(className);
			for (const Echo::PropertyInfo& prop : propertys)
			{
				m_propertyHelper.addItem(prop.m_name.c_str(), Echo::Vector3(), QT_UI::WT_Vector3);
			}
		}
		m_propertyHelper.endMenu();
	}

	// 属性修改后,更新结点值
	void NodeTreePanel::refreshPropertyToNode(const QString& property, QVariant value)
	{
		int a = 10;
	}
}