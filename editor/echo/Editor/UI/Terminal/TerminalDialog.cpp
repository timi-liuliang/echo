#include "Studio.h"
#include "TerminalDialog.h"
#include "EchoEngine.h"
#include "NodeTreePanel.h"
#include <engine/core/base/class.h>
#include <engine/core/main/module.h>
#include <engine/core/terminal/terminal.h>

namespace Studio
{
	TerminalDialog::TerminalDialog(QWidget* parent)
		: QDialog(parent)
	{
		setupUi(this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		QObject::connect(m_commandLineEdit, SIGNAL(returnPressed()), this, SLOT(onExec()));
	}

	TerminalDialog::~TerminalDialog()
	{

	}

	TerminalDialog* TerminalDialog::instance()
	{
		TerminalDialog* inst = new TerminalDialog();
		return inst;
	}

	void TerminalDialog::onExec()
	{
		Echo::String cmd = m_commandLineEdit->text().toStdString().c_str();
		Echo::Terminal::instance()->execCmd(cmd);
	}
}
