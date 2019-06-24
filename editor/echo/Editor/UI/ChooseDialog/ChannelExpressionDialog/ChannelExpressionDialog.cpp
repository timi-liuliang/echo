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
        
        // connect signal slot
        QObject::connect(m_textEditExression, SIGNAL(textChanged()), this, SLOT(onExpressionChanged()));
	}

	ChannelExpressionDialog::~ChannelExpressionDialog()
	{

	}

	bool ChannelExpressionDialog::getExpression(QWidget* parent, Echo::String& expression)
	{
		ChannelExpressionDialog dialog(parent);
        dialog.setExpressionText(expression);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			expression = dialog.getExpressionText();
            
            return true;
		}
		else
		{
			return false;
		}
	}
    
    const Echo::String ChannelExpressionDialog::getExpressionText() const
    {
        return m_textEditExression->toPlainText().toStdString().c_str();
    }
    
    void ChannelExpressionDialog::setExpressionText(const Echo::String& functionName)
    {
		m_textEditExression->setText(functionName.c_str());
    }
    
    void ChannelExpressionDialog::onExpressionChanged()
    {
        Echo::String expression = getExpressionText();
        m_ok->setEnabled(!expression.empty());
    }
}
