#include "shader_node_layer_blend.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderNodeLayerBlend::ShaderNodeLayerBlend()
	{
		m_outputs.resize(m_maxOutputNumber);
		for (Echo::i32 i = 0; i < m_maxOutputNumber; i++)
		{
			m_outputs[i] = std::make_shared<DataFloat>(this, Echo::StringUtil::Format("Layer_%d", i));
			m_outputs[i]->setVariableName(Echo::StringUtil::Format("%s[%d]", "v_Weight", i));
		}
	}

	void ShaderNodeLayerBlend::bindMethods()
	{

	}

	bool ShaderNodeLayerBlend::generateCode(Echo::ShaderCompiler& compiler)
	{
		compiler.addMacro("ENABLE_VERTEX_BLENDING");

		//compiler.addCode("\n");
		//compiler.addCode(Echo::StringUtil::Format("\tfloat %s[16] = float[16](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);\n", getVariableName().c_str()));

		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.x)] = v_Weight.x;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.y)] = v_Weight.y;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.z)] = v_Weight.z;\n", getVariableName().c_str()));
		//compiler.addCode(Echo::StringUtil::Format("\t%s[int(v_Joint.w)] = v_Weight.w;\n", getVariableName().c_str()));

		//compiler.addCode("\n");

		//float number = Echo::StringUtil::ParseFloat(m_lineEdit->text().toStdString().c_str());
		//compiler.addCode(Echo::StringUtil::Format("\tfloat %s = %f;\n", getVariableName().c_str(), number));

		return true;
	}
}

#endif
