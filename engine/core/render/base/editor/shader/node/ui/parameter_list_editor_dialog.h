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
		ParamterListEditorDialog(QWidget* parent, const String& params);
		virtual ~ParamterListEditorDialog();

		// get parameters
		String getParms();

		// parse
		void parse(const String& params);

		// add
		void add(const String& name, const String& type);

	protected:
		// slots
		void onAdd();
		void onDelete();
		void onChanged();
		void onMoveUp();
		void onMoveDown();
		void onCancel();

	private:
		// switch cell widget
		void switchCellWidget(i32 fromRow, i32 fromColumn, i32 toRow, i32 toColumn);

	protected:
		QStatusBar*		m_statusBar = nullptr;
		StringArray		m_types = {"float", "vec2", "vec3", "vec4", "sampler2D"};
	};
}
#endif

