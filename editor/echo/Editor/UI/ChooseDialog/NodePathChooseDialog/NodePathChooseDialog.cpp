#include "NodePathChooseDialog.h"
#include "NodeTreePanel.h"

namespace Studio
{
	NodePathChooseDialog::NodePathChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// update node tree widget
		NodeTreePanel::refreshNodeTreeDisplay(m_treeWidget);
	}

	NodePathChooseDialog::~NodePathChooseDialog()
	{

	}

	Echo::String NodePathChooseDialog::getSelectingNode(QWidget* parent)
	{
		NodePathChooseDialog dialog(parent);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			return dialog.getSelectingNodePath();
		}
		else
		{
			return Echo::StringUtil::BLANK;
		}
	}

	const Echo::String NodePathChooseDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
}