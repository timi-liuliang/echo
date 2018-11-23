#include "NodePathChooseDialog.h"

namespace Studio
{
	NodePathChooseDialog::NodePathChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	}

	NodePathChooseDialog::~NodePathChooseDialog()
	{

	}

	Echo::String NodePathChooseDialog::getSelectingNode(QWidget* parent)
	{
		static NodePathChooseDialog dialog(parent);
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
		return Echo::StringUtil::BLANK;
	}
}