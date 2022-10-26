#include "shader_node_texture.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/render/base/shader/editor/data/shader_data_texture.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeTexture::ShaderNodeTexture()
		: ShaderNode()
	{
		m_toolButton = new QToolButton();
		m_toolButton->setFixedSize(75, 75);

		m_inputDataTypes =
		{
			{"vec2", "uv"},
		};

		m_inputs.resize(m_inputDataTypes.size());

		m_outputs.resize(3);
		m_outputs[0] = std::make_shared<DataSampler2D>(this, "tex");
		m_outputs[1] = std::make_shared<DataVector3>(this, "rgb");
		m_outputs[2] = std::make_shared<DataFloat>(this, "a");

		updateOutputDataVariableName();
	}

	ShaderNodeTexture::~ShaderNodeTexture()
	{
	}

	void ShaderNodeTexture::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeTexture, setType);
		CLASS_BIND_METHOD(ShaderNodeTexture, getType);
		CLASS_BIND_METHOD(ShaderNodeTexture, setAtla);
		CLASS_BIND_METHOD(ShaderNodeTexture, isAtla);
		CLASS_BIND_METHOD(ShaderNodeTexture, setTexture);
		CLASS_BIND_METHOD(ShaderNodeTexture, getTexture);

		CLASS_REGISTER_PROPERTY(ShaderNodeTexture, "Type", Variant::Type::StringOption, getType, setType);
		CLASS_REGISTER_PROPERTY(ShaderNodeTexture, "Atla", Variant::Type::Bool, isAtla, setAtla);
		CLASS_REGISTER_PROPERTY(ShaderNodeTexture, "Texture", Variant::Type::ResourcePath, getTexture, setTexture);
	}

	void ShaderNodeTexture::setAtla(bool isAtla)
	{
		m_isAtla = isAtla;
		m_texture = ResourcePath(m_texture.getPath(), m_isAtla ? ".atla|.png|.rt" : ".png|.rt");
	}

	void ShaderNodeTexture::updateOutputDataVariableName()
	{
		Echo::String variableName = getVariableName();

		m_outputs[0]->setVariableName(variableName.c_str());
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s_Color.rgb", variableName.c_str()));
		m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s_Color.a", variableName.c_str()));

		Q_EMIT dataUpdated(0);
		Q_EMIT dataUpdated(1);
		Q_EMIT dataUpdated(2);
	}

	void ShaderNodeTexture::setInData(std::shared_ptr<NodeData> nodeData, int portIndex)
	{
		m_inputs[portIndex] = std::dynamic_pointer_cast<ShaderData>(nodeData);

		Q_EMIT dataUpdated(0);
		Q_EMIT dataUpdated(1);
		Q_EMIT dataUpdated(2);
	}

	bool ShaderNodeTexture::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		uniformNames.emplace_back("Uniforms." + getVariableName());
		uniformValues.emplace_back(m_texture);

		if (m_isAtla)
		{
			uniformNames.emplace_back("Uniforms." + getVariableName() + "Viewport");
			uniformValues.emplace_back(Echo::Color(0.f, 0.f, 1.f, 1.f));
		}

		return true;
	}

	bool ShaderNodeTexture::generateCode(ShaderCompiler& compiler)
	{
		updateOutputDataVariableName();

		compiler.addMacro("ENABLE_VERTEX_UV0");

		compiler.addTextureUniform(getVariableName());

		Echo::String uvConvertCode;
		if (isAtla())
		{
			compiler.addUniform("vec4", Echo::StringUtil::Format("%sViewport", getVariableName().c_str()));
			uvConvertCode = Echo::StringUtil::Format(" * fs_ubo.%sViewport.zw + fs_ubo.%sViewport.xy", getVariableName().c_str(), getVariableName().c_str());
		}

		if (isAtla())
		{
			compiler.addUniform("vec4", Echo::StringUtil::Format("%sViewport", getVariableName().c_str()));
			uvConvertCode = Echo::StringUtil::Format(" * fs_ubo.%sViewport.zw + fs_ubo.%sViewport.xy", getVariableName().c_str(), getVariableName().c_str());
		}

		if (getOutputConnectionCount(1) || getOutputConnectionCount(2))
		{
			if (m_inputs[0])
			{
				compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Color = texture( %s, %s %s);\n", getVariableName().c_str(), getVariableName().c_str(), dynamic_cast<ShaderData*>(m_inputs[0].get())->getVariableName().c_str(), uvConvertCode.c_str()));
			}
			else
			{
				compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Color = texture( %s, v_UV %s);\n", getVariableName().c_str(), getVariableName().c_str(), uvConvertCode.c_str()));
			}

			if (m_type.getValue() == "NormalMap")
			{
				compiler.addMacro("ENABLE_VERTEX_NORMAL");

				compiler.addCode(Echo::StringUtil::Format("\t%s_Color.rgb = _NormalMapFun(%s_Color.rgb);\n", getVariableName().c_str(), getVariableName().c_str()));
			}
		}

		return true;
	}

#endif
}

