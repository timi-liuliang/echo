#include "Document.h"
#include "EchoEngine.h"
#include <engine/core/base/class.h>

namespace Studio
{
	DocumentDialog::DocumentDialog(QWidget* parent)
		: QDialog(parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// display nodes
		initApiDisplay();

		// connect signal slot
		QObject::connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onSelectClass()));
	}

	DocumentDialog::~DocumentDialog()
	{

	}

	// instance
	DocumentDialog* DocumentDialog::instance()
	{
		DocumentDialog* inst = new DocumentDialog();
		return inst;
	}

	void DocumentDialog::initApiDisplay()
	{
		m_treeWidget->clear();

		// begin with "Node"
		addClassNode("Node", m_treeWidget->invisibleRootItem());

		// expand all items
		m_treeWidget->expandAll();
	}

	void DocumentDialog::addClassNode(const Echo::String& nodeName, QTreeWidgetItem* parent)
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
		if( Echo::Class::getChildClasses(childNodes, nodeName.c_str(), false))
		{
			for (const Echo::String& childNode : childNodes)
			{
				addClassNode(childNode, nodeItem);
			}
		}
	}

	void DocumentDialog::onSelectClass()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item)
		{
			Echo::String text = item->text(0).toStdString().c_str();
			//m_confirm->setEnabled(!Echo::Class::isVirtual(text));
		}
	}
}