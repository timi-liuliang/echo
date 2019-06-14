#include "SlotChooseDialog.h"
#include "NodeTreePanel.h"

namespace Studio
{
	SlotChooseDialog::SlotChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// update node tree widget
		NodeTreePanel::refreshNodeTreeDisplay(m_treeWidget);
        
        // connect signal slot
        QObject::connect(m_functionNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(onFunctionNameChanged()));
	}

	SlotChooseDialog::~SlotChooseDialog()
	{

	}

	bool SlotChooseDialog::getSlot(QWidget* parent, Echo::String& nodePath, Echo::String& functionName)
	{
		SlotChooseDialog dialog(parent);
        dialog.setFunctionName( functionName);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			nodePath = dialog.getSelectingNodePath();
			functionName = dialog.getFunctionName();
            
            return true;
		}
		else
		{
			return false;
		}
	}

	const Echo::String SlotChooseDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
    
    const Echo::String SlotChooseDialog::getFunctionName() const
    {
        return m_functionNameLineEdit->text().toStdString().c_str();
    }
    
    void SlotChooseDialog::setFunctionName(const Echo::String& functionName)
    {
        m_functionNameLineEdit->setText(functionName.c_str());
    }
    
    void SlotChooseDialog::onFunctionNameChanged()
    {
        Echo::String functionName = getFunctionName();
        m_ok->setEnabled(!functionName.empty());
    }
}
