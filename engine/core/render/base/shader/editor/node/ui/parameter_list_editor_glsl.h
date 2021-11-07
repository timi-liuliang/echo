#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/editor/property_editor.h"
#include "engine/core/editor/qt/QWidgets.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class ParamterListEditorGLSL : public PropertyEditor
	{
	public:
		ParamterListEditorGLSL();
		virtual ~ParamterListEditorGLSL();

		// set
		virtual void setObject(Object* object) override;

	private:
		// on text changed
		void onTextChanged();

		// on edit parameters
		void onEditParameters();

	protected:
		QHBoxLayout*	m_horizonLayout = nullptr;
		QLineEdit*		m_lineEdit = nullptr;
		QToolButton*	m_toolButton = nullptr;
	};
}
#endif

