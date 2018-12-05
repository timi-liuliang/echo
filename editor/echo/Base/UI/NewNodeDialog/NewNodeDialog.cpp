#include "Studio.h"
#include "NewNodeDialog.h"
#include "EchoEngine.h"
#include "NodeTreePanel.h"
#include <engine/core/base/class.h>

namespace Studio
{
	NewNodeDialog::NewNodeDialog(QWidget* parent)
		: QDialog(parent)
		, m_viewNodeByModule(true)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// connect signal slot
		QObject::connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onSelectNode()));
		QObject::connect(m_confirm, SIGNAL(clicked()), this, SLOT(onConfirmNode()));
		QObject::connect(m_confirm, SIGNAL(clicked()), this, SLOT(accept()));
		QObject::connect(m_cancel, SIGNAL(clicked()), this, SLOT(reject()));
		QObject::connect(m_viewNodeButton, SIGNAL(clicked()), this, SLOT(onSwitchNodeVeiwType()));
		QObject::connect(m_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(onConfirmNode()));

		recoverEditSettings();
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

	void NewNodeDialog::initNodeDisplayByModule()
	{
		m_treeWidget->clear();

		addNode( "Node");

		m_treeWidget->expandAll();
	}

	QTreeWidgetItem* NewNodeDialog::getModuleItem(const Echo::String& nodeName)
	{
		Echo::ClassInfo* cinfo = Echo::Class::getClassInfo(nodeName);
		Echo::String moduleName = cinfo ? cinfo->m_module : "Core";

		QTreeWidgetItem* rootItem = m_treeWidget->invisibleRootItem();
		for (int i = 0; i < rootItem->childCount(); i++)
		{
			QTreeWidgetItem* moduleItem = rootItem->child(i);
			if (moduleItem->text(0).toStdString().c_str() == moduleName)
			{
				return moduleItem;
			}
		}

		// create module item
		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(rootItem);
		nodeItem->setText(0, moduleName.c_str());
		nodeItem->setIcon(0, QIcon(":/icon/node/box2dbody.png"));

		return getModuleItem( moduleName);
	}

	// create QTreewidgetItem by nodename
	QTreeWidgetItem* NewNodeDialog::createQTreeWidgetItemByNodeName(const Echo::String& nodeName, QTreeWidgetItem* parent)
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
		nodeItem->setText(0, nodeName.c_str());
		nodeItem->setIcon(0, QIcon(iconPath.empty() ? qIconPath.c_str() : (rootPath + iconPath).c_str()));

		return nodeItem;
	}

	void NewNodeDialog::addNode(const Echo::String& nodeName)
	{
		// use module item as parent
		QTreeWidgetItem* moduleParent = getModuleItem(nodeName);
		createQTreeWidgetItemByNodeName(nodeName, moduleParent);

		// recursive all children
		Echo::StringArray childNodes;
		if (Echo::Class::getChildClasses(childNodes, nodeName.c_str(), false))
		{
			for (const Echo::String& childNode : childNodes)
			{
				addNode(childNode);
			}
		}
	}

	void NewNodeDialog::initNodeDisplayByInherite()
	{
		m_treeWidget->clear();

		// begin with "Node"
		addNode("Node", m_treeWidget->invisibleRootItem());

		// expand all items
		m_treeWidget->expandAll();
	}

	void NewNodeDialog::addNode(const Echo::String& nodeName, QTreeWidgetItem* parent)
	{
		// create by node name
		QTreeWidgetItem* nodeItem = createQTreeWidgetItemByNodeName(nodeName, parent);

		// recursive all children
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

	void NewNodeDialog::onSwitchNodeVeiwType()
	{
		m_viewNodeByModule = !m_viewNodeByModule;
		if (m_viewNodeByModule)
		{
			initNodeDisplayByModule();

			m_viewNodeButton->setIcon(QIcon(":/icon/Icon/res/view_type_list.png"));
			m_viewNodeButton->setToolTip("Display node by Module");
		}
		else
		{
			initNodeDisplayByInherite();

			m_viewNodeButton->setIcon(QIcon(":/icon/Icon/res/view_type_grid.png"));
			m_viewNodeButton->setToolTip("Display node by inheritance relationships");
		}

		// save config
		AStudio::instance()->getConfigMgr()->setValue("NewNodeDialog_NodeViewType", m_viewNodeByModule ? "Module" : "Inherit");
	}

	void NewNodeDialog::recoverEditSettings()
	{
		Echo::String viewType = AStudio::instance()->getConfigMgr()->getValue("NewNodeDialog_NodeViewType");
		if (!viewType.empty() && m_viewNodeByModule && viewType != "Module")
		{
			onSwitchNodeVeiwType();
		}
		else
		{
			initNodeDisplayByModule();
		}
	}
}