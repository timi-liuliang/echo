#pragma once

#include "engine/core/base/object.h"
#include "../compiler/shader_compiler.h"
#include <functional>

#ifdef ECHO_EDITOR_MODE

#include <nodeeditor/NodeData>

namespace Echo
{
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

	public:
		// get input data types
		virtual QtNodes::NodeDataTypes getInputDataTypes() { return QtNodes::NodeDataTypes(); }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) { return false; }

	public:
		// signals
		void setInputDataTypesChangedCb(std::function<void()> cb) { m_inputDataTypesChangedCb = cb; }		

	protected:
		String					m_caption;
		std::function<void()>	m_inputDataTypesChangedCb;
	};
}

#endif
