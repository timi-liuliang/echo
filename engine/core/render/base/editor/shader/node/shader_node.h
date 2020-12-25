#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNode : public Object
	{
		ECHO_CLASS(ShaderNode, Object)

	public:
		ShaderNode();
		virtual ~ShaderNode();

		// name
		virtual const char* getName() const { return "Unknown"; }

		// caption
		const String& getCaption() const { return m_caption; }
		void setCaption(const String& caption) { m_caption = caption; }

	protected:
		String			m_caption;
	};

#endif
}
