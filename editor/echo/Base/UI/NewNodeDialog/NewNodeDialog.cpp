#include "Studio.h"
#include "NewNodeDialog.h"
#include "EchoEngine.h"
#include "NodeTreePanel.h"
#include <engine/core/base/class.h>

namespace Studio
{
	NewNodeDialog::NewNodeDialog(QWidget* parent)
		: QDialog(parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// display nodes
		initNodeDisplay();

		// connect signal slot
		QObject::connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onSelectNode()));
		QObject::connect(m_confirm, SIGNAL(clicked()), this, SLOT(onConfirmNode()));
		QObject::connect(m_confirm, SIGNAL(clicked()), this, SLOT(accept()));
		QObject::connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
	}

	NewNodeDialog::~NewNodeDialog()
	{

	}

	// instance
	NewNodeDialog* NewNodeDialog::instance()
	{
		NewNodeDialog* inst = new NewNodeDialog();
		return inst;
	}

	// get node type
	Echo::String NewNodeDialog::getSelectedNodeType()
	{
		NewNodeDialog* inst = instance();
		inst->setVisible(true);
		if (inst->exec() == QDialog::Accepted)
		{
			return inst->m_result;
		}

		return "";
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
		// iconpath
		Echo::Node* node = (Echo::Node*)Echo::Class::create(nodeName);
		Echo::String iconPath = node->getEditor() ? node->getEditor()->getEditorIcon() : "";
		EchoSafeDelete(node, Node);

		Echo::String rootPath = AStudio::instance()->getRootPath();

		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		Echo::String qIconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(parent);
		nodeItem->setText( 0, nodeName.c_str());
		nodeItem->setIcon(0, QIcon(iconPath.empty() ? qIconPath.c_str() : (rootPath + iconPath).c_str()));

		QObject::connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onConfirmNode()));

		Echo::StringArray childNodes;
		if( Echo::Class::getChildClasses(childNodes, nodeName.c_str(), false))
		{
			for (const Echo::String& childNode : childNodes)
			{
				addNode(childNode, nodeItem);
			}
		}
	}

	void NewNodeDialog::onSelectNode()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item)
		{
			Echo::String text = item->text(0).toStdString().c_str();
			m_confirm->setEnabled(!Echo::Class::isVirtual(text));
		}
	}

	void NewNodeDialog::onConfirmNode()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item)
		{
			Echo::String text = item->text(0).toStdString().c_str();	
			if (!Echo::Class::isVirtual( text))
			{
				m_result = text;

				hide();
			}
		}
	}
}