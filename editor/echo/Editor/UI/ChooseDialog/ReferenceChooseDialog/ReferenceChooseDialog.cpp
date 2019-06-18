#include "ReferenceChooseDialog.h"
#include "NodeTreePanel.h"
#include "PropertyChooseDialog.h"

namespace Studio
{
	ReferenceChooseDialog::ReferenceChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// update node tree widget
		NodeTreePanel::refreshNodeTreeDisplay(m_treeWidget);
        
        // connect signal slot
        QObject::connect(m_treeWidgetProperty, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(currentItemChanged()));
        QObject::connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(onClickedNodeItem()));
        
        // simulate event
        onClickedNodeItem();
	}

	ReferenceChooseDialog::~ReferenceChooseDialog()
	{

	}

	bool ReferenceChooseDialog::getReference(QWidget* parent, Echo::String& nodePath, Echo::String& propertyName)
	{
		ReferenceChooseDialog dialog(parent);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			nodePath = dialog.getSelectingNodePath();
			propertyName = dialog.getPropertyName();
            
            return true;
		}
		else
		{
			return false;
		}
	}

	const Echo::String ReferenceChooseDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
    
    const Echo::String ReferenceChooseDialog::getPropertyName() const
    {
		return m_propertyName;
    }
    
    void ReferenceChooseDialog::currentItemChanged()
    {
        QTreeWidgetItem* item = m_treeWidgetProperty->currentItem();
        if (item && item->data(0, Qt::UserRole).toString()=="property")
        {
            m_propertyName = item->text(0).toStdString().c_str();
            m_ok->setEnabled(true);
        }
        else
        {
            m_propertyName = "";
            m_ok->setEnabled(false);
        }
    }
    
    void ReferenceChooseDialog::onClickedNodeItem()
    {
        QTreeWidgetItem* item = m_treeWidget->currentItem();
        Echo::i32 nodeId = item->data(0, Qt::UserRole).toInt();
        Echo::Node* node = (Echo::Node*)Echo::Node::getById(nodeId);
        if(node)
        {
            m_treeWidgetProperty->clear();
            PropertyChooseDialog::refreshPropertysDisplayRecursive(m_treeWidgetProperty, node, node->getClassName());
            m_treeWidgetProperty->expandAll();
        }
    }
}
