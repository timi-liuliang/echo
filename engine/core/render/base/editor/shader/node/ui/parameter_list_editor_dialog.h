#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/editor/property_editor.h"
#include "engine/core/editor/qt/QWidgets.h"

#ifdef ECHO_EDITOR_MODE

#include "ui_parameter_list_editor_dialog.h"

namespace Echo
{
	class ParamterListEditorDialog : public QDialog, public Ui_ParameterListEditorDialog
	{
	public:
		ParamterListEditorDialog(QWidget* parent);
		virtual ~ParamterListEditorDialog();

	protected:
		// slots
		void onAdd();
		void onDelete();
		void onMoveUp();
		void onMoveDown();
		void onOk();
		void onCancel();

	protected:
		StringArray		m_types;
	};
}
#endif

