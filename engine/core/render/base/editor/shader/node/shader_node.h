#pragma once

#include "engine/core/base/object.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNode : public Object
	{
		ECHO_CLASS(ShaderNode, Object)

	public:
		// Data Type
		struct DataType
		{
			String m_type;
			String m_name;
		};
		typedef vector<DataType>::type DataTypes;

	public:
		ShaderNode();
		virtual ~ShaderNode();

		// name
		virtual const char* getName() const { return "Unknown"; }

		// caption
		const String& getCaption() const { return m_caption; }
		void setCaption(const String& caption) { m_caption = caption; }

	public:
		// get input data types
		virtual DataTypes getInputDataTypes() { return DataTypes(); }

	protected:
		String			m_caption;
	};

#endif
}
