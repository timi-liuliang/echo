#pragma once

#include <engine/core/util/StringUtil.h>
#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <engine/core/editor/qt/extension/QMenuBarEx.h>
#include "ui_TerminalDialog.h"

namespace Studio
{
	class TerminalDialog : public QDialog, public Ui_TerminalDialog
	{
		Q_OBJECT

	public:
		TerminalDialog(QWidget* parent = 0);
		~TerminalDialog();

		// instance
		static TerminalDialog* instance();

	public slots:
		// exec
		void onExec();
	};
}
