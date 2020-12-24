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

		// get parmeters
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
		void onOk();
		void onCancel();

	protected:
		QStatusBar*		m_statusBar = nullptr;
		StringArray		m_types = {"float", "vec2", "vec3", "vec4"};
	};
}
#endif

