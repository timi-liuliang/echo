#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "ScenePanel.h"
#include "EchoEngine.h"

namespace Studio
{
	static ScenePanel* g_inst = nullptr;

	ScenePanel::ScenePanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_newNodeDialog(nullptr)
	{
		EchoAssert(!g_inst);
		g_inst = this;

		setupUi( this);

		QObject::connect(m_newNodeButton, SIGNAL(clicked()), this, SLOT(showNewNodeDialog()));
	}

	ScenePanel::~ScenePanel()
	{

	}

	ScenePanel* ScenePanel::instance()
	{
		return g_inst;
	}

	// 设置显示配置
	void ScenePanel::setDisplayConfig(const char* config)
	{
		EchoSafeDelete(m_property, QProperty);

		m_property = EchoNew(QT_UI::QProperty(config, m_propertyTreeView));
		m_property->m_model->setObjectName(QStringLiteral("property_model"));
		m_propertyTreeView->setModel(m_property->m_model);
		m_propertyTreeView->setItemDelegate(m_property->m_delegate);
		m_propertyTreeView->setAlternatingRowColors(true);
		m_propertyTreeView->expandAll();
	}

	// 设置属性值
	void ScenePanel::setPropertyValue(const char* propertyName, QVariant& value)
	{
		m_property->m_model->setValue(propertyName, value);
	}

	void ScenePanel::showNewNodeDialog()
	{
		if (!m_newNodeDialog)
			m_newNodeDialog = new NewNodeDialog(this);

		m_newNodeDialog->setVisible(true);
	}


	void ScenePanel::refreshNodeTreeDisplay()
	{
		m_nodeTreeWidget->clear();

		// begin with "Node"
		addNode(EchoEngine::Instance()->getCurrentEditNode(), m_nodeTreeWidget->invisibleRootItem(), true);

		m_nodeTreeWidget->expandAll();
	}

	void ScenePanel::addNode(Echo::Node* node, QTreeWidgetItem* parent, bool recursive)
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

	Echo::Node* ScenePanel::getCurrentSelectNode()
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

	void ScenePanel::addNode(Echo::Node* node)
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
}