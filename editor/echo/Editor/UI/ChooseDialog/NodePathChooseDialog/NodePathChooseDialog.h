#pragma once

#include <QDialog>
#include "ui_NodePathChooseDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class NodePathChooseDialog : public QDialog, public Ui_NodePathChooseDialog
	{
		Q_OBJECT

	public:
		NodePathChooseDialog(QWidget* parent);
		virtual ~NodePathChooseDialog();

		// get selecting node
		static Echo::String getSelectingNode( QWidget* parent);

	private:
		// get selecting node path
		const Echo::String getSelectingNodePath() const;
	};
}