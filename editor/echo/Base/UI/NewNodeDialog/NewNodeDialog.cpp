#include "NewNodeDialog.h"
#include "EchoEngine.h"
#include "NodeTreePanel.h"
#include <engine/core/base/class.h>

namespace Studio
{
	NewNodeDialog::NewNodeDialog(QWidget* parent)
		: QMainWindow(parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// display nodes
		initNodeDisplay();

		QObject::connect(m_confirm, SIGNAL(clicked()), this, SLOT(onConfirmNode()));
	}

	NewNodeDialog::~NewNodeDialog()
	{

	}

	void NewNodeDialog::initNodeDisplay()
	{
		m_treeWidget->clear();

		// begin with "Node"
		addNode("Node", m_treeWidget->invisibleRootItem());

		// expand all items
		m_treeWidget->expandAll();
	}

	void NewNodeDialog::addNode(const Echo::String& nodeName, QTreeWidgetItem* parent)
	{
		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		Echo::String iconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(parent);
		nodeItem->setText( 0, nodeName.c_str());
		nodeItem->setIcon(0, QIcon(iconPath.c_str()));

		QObject::connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onConfirmNode()));

		Echo::StringArray childNodes;
		if( Echo::Class::getChildClasses(childNodes, nodeName.c_str()))
		{
			for (const Echo::String& childNode : childNodes)
			{
				addNode(childNode, nodeItem);
			}
		}
	}

	void NewNodeDialog::onConfirmNode()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item)
		{
			Echo::String text = item->text(0).toStdString().c_str();
			Echo::Node* node = Echo::Class::create<Echo::Node*>(text.c_str());
			if (node)
			{
				NodeTreePanel::instance()->addNode(node);
			}
		}

		// hide window
		hide();
	}
}