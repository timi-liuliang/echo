#pragma once

#include <QDialog>
#include "ui_ReferenceChooseDialog.h"
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class ReferenceChooseDialog : public QDialog, public Ui_ReferenceChooseDialog
	{
		Q_OBJECT

	public:
		ReferenceChooseDialog(QWidget* parent);
		virtual ~ReferenceChooseDialog();

		// get selecting node
        static bool getReference(QWidget* parent, Echo::String& nodePath, Echo::String& propertyName);

		// get selecting node path
		const Echo::String getSelectingNodePath() const;
        
        // function name
        const Echo::String getPropertyName() const;
        
    private slots:
        // on function name changed slot
        void currentItemChanged();
        
        // on click node item
        void onClickedNodeItem();
        
    private:
        Echo::String    m_propertyName;
	};
}
