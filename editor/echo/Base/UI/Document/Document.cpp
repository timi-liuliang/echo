#include "Document.h"
#include "EchoEngine.h"
#include <engine/core/base/class.h>

namespace Studio
{
	DocumentPanel::DocumentPanel(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi(this);

		// splitter stretch
		m_splitter->setStretchFactor(0, 0);
		m_splitter->setStretchFactor(1, 1);

		// display nodes
		initApiDisplay();

		// connect signal slot
		QObject::connect(m_treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onSelectClass()));
	}

	DocumentPanel::~DocumentPanel()
	{

	}

	void DocumentPanel::initApiDisplay()
	{
		m_treeWidget->clear();

		// api items
		QTreeWidgetItem* apiItem = new QTreeWidgetItem(m_treeWidget->invisibleRootItem());
		apiItem->setText(0, "Api");

		// begin with "Object"
		addClassNode("Object", apiItem);

		// expand all items
		m_treeWidget->expandAll();
	}

	void DocumentPanel::addClassNode(const Echo::String& nodeName, QTreeWidgetItem* parent)
	{
		// get icon path by node name
		Echo::String lowerCaseNodeName = nodeName;
		Echo::StringUtil::LowerCase(lowerCaseNodeName);
		//Echo::String iconPath = Echo::StringUtil::Format(":/icon/node/%s.png", lowerCaseNodeName.c_str());

		QTreeWidgetItem* nodeItem = new QTreeWidgetItem(parent);
		nodeItem->setText( 0, nodeName.c_str());
		//nodeItem->setIcon(0, QIcon(iconPath.c_str()));

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

	void DocumentPanel::onSelectClass()
	{
		QTreeWidgetItem* item = m_treeWidget->currentItem();
		if (item)
		{
			Echo::String className = item->text(0).toStdString().c_str();

			// iterator all methods
			Echo::StringArray methods;
			Echo::LuaBinder::instance()->getClassMethods(className, methods);

			// organize display
			Echo::String text;
			text += "<font color=\"green\">" + className + "</font>";
			text += "<br />";
			for (const Echo::String& methodName : methods)
			{
				text += "<br />";
				text += methodName;
			}

			m_textBrowser->setHtml(text.c_str());
		}
	}
}