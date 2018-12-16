#pragma once

#include <QDialog>
#include "ui_PropertyChooseDialog.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/base/object.h>

namespace Studio
{
	class PropertyChooseDialog : public QDialog, public Ui_PropertyChooseDialog
	{
		Q_OBJECT

	public:
		PropertyChooseDialog(QWidget* parent, Echo::Object* objectPtr);
		virtual ~PropertyChooseDialog();

		// get selecting node
		static Echo::String getSelectingProperty( QWidget* parent, Echo::Object* objectPtr);

	private:
		// get selecting node path
		const Echo::String getSelectingProperty() const;

		// refresh propertys display
		void refreshPropertysDisplayRecursive(Echo::Object* objectPtr, const Echo::String& className);

	private slots:
		// on current item changed
		void currentItemChanged();
	};
}