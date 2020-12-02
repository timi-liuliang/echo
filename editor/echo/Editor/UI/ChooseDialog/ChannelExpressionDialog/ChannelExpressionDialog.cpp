#include "ChannelExpressionDialog.h"
#include "NodeTreePanel.h"
#include "ReferenceChooseDialog.h"

namespace Studio
{
	ChannelExpressionDialog::ChannelExpressionDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

#ifdef ECHO_PLATFORM_WINDOWS
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		m_menuBar->setNativeMenuBar(false);
#endif

		m_menuBar->setCornderButtonVisible(QT_UI::QMenuBarEx::Minus, false);
		m_menuBar->setCornderButtonVisible(QT_UI::QMenuBarEx::Minimize, false);
		m_menuBar->setCornderButtonVisible(QT_UI::QMenuBarEx::FullScreen, false);
        
        // connect signal slot
        QObject::connect(m_textEditExression, SIGNAL(textChanged()), this, SLOT(onExpressionChanged()));
		QObject::connect(m_referenceNodePropertyButton, SIGNAL(clicked()), this, SLOT(referenceNodeProperty()));
	}

	ChannelExpressionDialog::~ChannelExpressionDialog()
	{

	}

	bool ChannelExpressionDialog::getExpression(QWidget* parent, Echo::String& expression, Echo::ui32 currentObjId)
	{
		ChannelExpressionDialog dialog(parent);
        dialog.setExpressionText(expression);
		dialog.setObject(Echo::Object::getById(currentObjId));
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

	void ChannelExpressionDialog::setObject(Echo::Object* object)
	{ 
		m_currentObject = object;
		m_referenceNodePropertyButton->setVisible(dynamic_cast<Echo::Node*>(m_currentObject));
	}
    
    void ChannelExpressionDialog::onExpressionChanged()
    {
        Echo::String expression = getExpressionText();
        m_ok->setEnabled(!expression.empty());
    }

	void ChannelExpressionDialog::referenceNodeProperty()
	{
		Echo::String nodePath;
		Echo::String propertyName;
		if (ReferenceChooseDialog::getReference(this, nodePath, propertyName))
		{
			Echo::Node* currentNode = dynamic_cast<Echo::Node*>(m_currentObject);
	        if(currentNode)
	        {
				Echo::Node* fromNode = currentNode->getNode(nodePath.c_str());
	            Echo::String relativePath = fromNode->getNodePathRelativeTo(currentNode);
	  
	            Echo::String expression = Echo::StringUtil::Format("ch(\"%s\", \"%s\")", relativePath.c_str(), propertyName.c_str());
				setExpressionText(expression);
	        }
		}
	}
}
