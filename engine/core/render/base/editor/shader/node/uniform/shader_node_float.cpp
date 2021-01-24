#include "shader_node_float.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeFloat::ShaderNodeFloat()
      : ShaderNodeUniform()
    {
		setupWidgets();

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataFloat>(this, "float");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
    }

	ShaderNodeFloat::~ShaderNodeFloat()
	{
	}

	void ShaderNodeFloat::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeFloat, getValue, DEF_METHOD("getValue"));
		CLASS_BIND_METHOD(ShaderNodeFloat, setValue, DEF_METHOD("setValue"));

		CLASS_REGISTER_PROPERTY(ShaderNodeFloat, "Value", Variant::Type::Real, "getValue", "setValue");
	}

	void ShaderNodeFloat::setVariableName(const String& variableName)
	{
		m_variableName = variableName;
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));

		Q_EMIT dataUpdated(0);
	}

	void ShaderNodeFloat::setValue(float value)
	{
		m_value = value;
		m_lineEdit->setText(StringUtil::ToString(m_value).c_str());
	}

	bool ShaderNodeFloat::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_isUniform)
		{
			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(m_value);

			return true;
		}

		return false;
	}

    bool ShaderNodeFloat::generateCode(Echo::ShaderCompiler& compiler)
    {
        if (m_isUniform)
        {
            compiler.addUniform("float", getVariableName().c_str());

			compiler.addCode(Echo::StringUtil::Format("\tfloat %s_Value = fs_ubo.%s;\n", getVariableName().c_str(), getVariableName().c_str()));
        }
        else
        {
			compiler.addCode(Echo::StringUtil::Format("\tfloat %s_Value = %f;\n", getVariableName().c_str(), m_value));
        }

		return true;
    }

	void ShaderNodeFloat::setupWidgets()
	{
		m_lineEdit = new QLineEdit();
		m_lineEdit->setValidator(new QDoubleValidator());
		m_lineEdit->setMaximumSize(QSize(m_lineEdit->sizeHint().width() * 0.4f, m_lineEdit->sizeHint().height()));
		m_lineEdit->setText("0.0");

		EditorApi.qConnectWidget(m_lineEdit, QSIGNAL(editingFinished()), this, createMethodBind(&ShaderNodeFloat::onTextEdited));
	}

	void ShaderNodeFloat::onTextEdited()
	{
		m_value = StringUtil::ParseFloat(m_lineEdit->text().toStdString().c_str());
	}
}

#endif
