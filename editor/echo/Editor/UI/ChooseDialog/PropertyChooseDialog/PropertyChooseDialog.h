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
        
        // refresh propertys display
        static void refreshPropertysDisplayRecursive(QTreeWidget* treeWidget, Echo::Object* objectPtr, const Echo::String& className);

	private:
		// get selecting node path
		const Echo::String getSelectingProperty() const;

	private slots:
		// on current item changed
		void currentItemChanged();
	};
}
