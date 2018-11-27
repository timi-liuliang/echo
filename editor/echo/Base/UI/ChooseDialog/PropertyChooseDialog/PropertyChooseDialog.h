#pragma once

#include <QDialog>
#include "ui_PropertyChooseDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class PropertyChooseDialog : public QDialog, public Ui_PropertyChooseDialog
	{
		Q_OBJECT

	public:
		PropertyChooseDialog(QWidget* parent);
		virtual ~PropertyChooseDialog();

		// get selecting node
		static Echo::String getSelectingNode( QWidget* parent);

	private:
		// get selecting node path
		const Echo::String getSelectingNodePath() const;
	};
}