#include "ReferenceChooseDialog.h"
#include "NodeTreePanel.h"

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
        //QObject::connect(m_functionNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onFunctionNameChanged()));
	}

	ReferenceChooseDialog::~ReferenceChooseDialog()
	{

	}

	bool ReferenceChooseDialog::getReference(QWidget* parent, Echo::String& nodePath, Echo::String& propertyName)
	{
		ReferenceChooseDialog dialog(parent);
        dialog.setFunctionName(propertyName);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			nodePath = dialog.getSelectingNodePath();
			propertyName = dialog.getFunctionName();
            
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
    
    const Echo::String ReferenceChooseDialog::getFunctionName() const
    {
		return "";// m_functionNameLineEdit->text().toStdString().c_str();
    }
    
    void ReferenceChooseDialog::setFunctionName(const Echo::String& functionName)
    {
        //m_functionNameLineEdit->setText(functionName.c_str());
    }
    
    void ReferenceChooseDialog::onFunctionNameChanged()
    {
        Echo::String functionName = getFunctionName();
        m_ok->setEnabled(!functionName.empty());
    }
}
