#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/editor/property_editor.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class ParamterListEditorGLSL : public PropertyEditor
	{
	public:
		ParamterListEditorGLSL();
		virtual ~ParamterListEditorGLSL();

	protected:
	};
}
#endif

