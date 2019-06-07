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
	}

	SlotChooseDialog::~SlotChooseDialog()
	{

	}

	Echo::String SlotChooseDialog::getSelectingNode(QWidget* parent)
	{
		SlotChooseDialog dialog(parent);
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

	const Echo::String SlotChooseDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
}
