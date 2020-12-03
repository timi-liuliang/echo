#pragma once

#include <QDialog>
#include "LuaSyntaxHighLighter.h"
#include "ui_ChannelExpressionDialog.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/base/object.h>

namespace Studio
{
	class ChannelExpressionDialog : public QDialog, public Ui_ChannelExpressionDialog
	{
		Q_OBJECT

	public:
		ChannelExpressionDialog(QWidget* parent);
		virtual ~ChannelExpressionDialog();

		// get selecting node
        static bool getExpression(QWidget* parent, Echo::String& expression, Echo::ui32 currentObjId);
        
        // function name
        const Echo::String getExpressionText() const;
        void setExpressionText(const Echo::String& functionName);

        // object
        void setObject(Echo::Object* object);
        
    private slots:
        // on function name changed slot
        void onExpressionChanged();

        // reference node's property
        void referenceNodeProperty();

    private:
        Echo::Object*           m_currentObject = nullptr;
        LuaSyntaxHighLighter*   m_luaSyntaxHighLighter = nullptr;
	};
}
