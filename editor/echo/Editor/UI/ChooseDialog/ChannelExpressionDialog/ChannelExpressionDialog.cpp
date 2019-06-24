#include "ChannelExpressionDialog.h"
#include "NodeTreePanel.h"

namespace Studio
{
	ChannelExpressionDialog::ChannelExpressionDialog(QWidget* parent)
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

	ChannelExpressionDialog::~ChannelExpressionDialog()
	{

	}

	bool ChannelExpressionDialog::getExpression(QWidget* parent, Echo::String& expression)
	{
		ChannelExpressionDialog dialog(parent);
        dialog.setFunctionName(expression);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			//nodePath = dialog.getSelectingNodePath();
			//functionName = dialog.getFunctionName();
            
            return true;
		}
		else
		{
			return false;
		}
	}

	const Echo::String ChannelExpressionDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
    
    const Echo::String ChannelExpressionDialog::getFunctionName() const
    {
        return m_functionNameLineEdit->text().toStdString().c_str();
    }
    
    void ChannelExpressionDialog::setFunctionName(const Echo::String& functionName)
    {
        m_functionNameLineEdit->setText(functionName.c_str());
    }
    
    void ChannelExpressionDialog::onFunctionNameChanged()
    {
        Echo::String functionName = getFunctionName();
        m_ok->setEnabled(!functionName.empty());
    }
}
