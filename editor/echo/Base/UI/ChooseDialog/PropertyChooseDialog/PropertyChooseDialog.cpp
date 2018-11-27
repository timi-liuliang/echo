#include "PropertyChooseDialog.h"
#include "NodeTreePanel.h"

namespace Studio
{
	PropertyChooseDialog::PropertyChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// update node tree widget
		NodeTreePanel::refreshNodeTreeDisplay(m_treeWidget);
	}

	PropertyChooseDialog::~PropertyChooseDialog()
	{

	}

	Echo::String PropertyChooseDialog::getSelectingNode(QWidget* parent)
	{
		return Echo::StringUtil::BLANK;
	}

	const Echo::String PropertyChooseDialog::getSelectingNodePath() const
	{
		Echo::Node* node = NodeTreePanel::getNode(m_treeWidget->currentItem());
		return node ? node->getNodePath() : Echo::StringUtil::BLANK;
	}
}