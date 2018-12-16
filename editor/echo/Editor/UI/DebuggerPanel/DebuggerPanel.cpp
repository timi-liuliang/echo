#include "DebuggerPanel.h"
#include <QListWidgetItem>

namespace Studio
{
	// 构造函数
	DebuggerPanel::DebuggerPanel(QWidget* parent/* = 0*/)
		: QWidget( parent)
	{
		setupUi(this);
	}

	// 析构函数
	DebuggerPanel::~DebuggerPanel()
	{
	}
}