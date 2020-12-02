#pragma once

#include "object_editor.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class NodeEditor : public ObjectEditor
	{
	public:
		NodeEditor(Object* object) : ObjectEditor(object) {}
		virtual ~NodeEditor() {}

	public:
		// transform widget
		virtual bool isUseTransformWidget() { return m_isSelected; }

		// get transform widget position
		virtual const Vector3& getTransformWidgetPosition() { return Vector3::ZERO; }

		// on operate transform widget
		virtual bool onOperateTransformWidget(const Vector3& translate, float yaw, float pitch, float roll, const Vector3& scale) { return false; }
	};
}
#endif