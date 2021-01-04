#include "TextureDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>

using namespace Echo;

namespace DataFlowProgramming
{
    TextureDataModel::TextureDataModel()
        : ShaderUniformDataModel()
    {
        m_uniformConfig = EchoNew(Echo::ShaderNodeUniformTexture);
        m_uniformConfig->setExport(true);
        m_uniformConfig->setVariableName(getDefaultVariableName());

        m_textureSelect = new QT_UI::QTextureSelect();
        m_textureSelect->setFixedSize(128, 128);

        QObject::connect(m_textureSelect, SIGNAL(Signal_TextureChagned()), this, SLOT(onTextureEdited()));

	    m_inputDataTypes =
	    {
		    {"vec2", "UV"},
	    };

	    m_inputs.resize(m_inputDataTypes.size());

	    m_outputs.resize(5);
	    m_outputs[0] = std::make_shared<DataVector3>(this, "rgb");
	    m_outputs[1] = std::make_shared<DataFloat>(this, "r");
	    m_outputs[2] = std::make_shared<DataFloat>(this, "g");
	    m_outputs[3] = std::make_shared<DataFloat>(this, "b");
	    m_outputs[4] = std::make_shared<DataFloat>(this, "a");

        updateOutputDataVariableName();
    }

    QJsonObject TextureDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

        ShaderUniformDataModel::saveUniformConfig(modelJson);

        modelJson["texture"] = m_textureSelect->getTexture().c_str();
        modelJson["isAtla"] = Echo::StringUtil::ToString( ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->isAtla()).c_str();
        modelJson["type"] = ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->getType().getValue().c_str();

        return modelJson;
    }

    void TextureDataModel::restore(QJsonObject const &p)
    {
        ShaderUniformDataModel::restoreUniformConfig(p);

        QJsonValue v = p["texture"];
        if (!v.isUndefined())
        {
            Echo::String texturePath = v.toString().toStdString().c_str();
            m_textureSelect->setTexture(texturePath);
        }

        v = p["isAtla"];
        if (!v.isUndefined())
        {
            Echo::String isAtlaStr = v.toString().toStdString().c_str();
            ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->setAtla(Echo::StringUtil::ParseBool(isAtlaStr));
        }

		v = p["type"];
		if (!v.isUndefined())
		{
			Echo::String type = v.toString().toStdString().c_str();
			ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->setType(type);
		}
    }

    void TextureDataModel::onTextureEdited()
    {
		Q_EMIT dataUpdated(0);
		Q_EMIT dataUpdated(1);
		Q_EMIT dataUpdated(2);
		Q_EMIT dataUpdated(3);
		Q_EMIT dataUpdated(4);
    }

	void TextureDataModel::setInData(std::shared_ptr<NodeData> nodeData, PortIndex port)
	{
		m_inputs[port] = std::dynamic_pointer_cast<ShaderData>(nodeData);
        
        onTextureEdited();
	}

	void TextureDataModel::updateOutputDataVariableName()
	{
		Echo::String variableName = getVariableName();

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Color.rgb", variableName.c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s_Color.r", variableName.c_str()));
		m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s_Color.g", variableName.c_str()));
		m_outputs[3]->setVariableName(Echo::StringUtil::Format("%s_Color.b", variableName.c_str()));
		m_outputs[4]->setVariableName(Echo::StringUtil::Format("%s_Color.a", variableName.c_str()));
	}

	bool TextureDataModel::generateCode(Echo::ShaderCompiler& compiler)
	{                   
        updateOutputDataVariableName();

		compiler.addMacro("ENABLE_VERTEX_UV0");

		compiler.addTextureUniform(getVariableName());

        Echo::String uvConvertCode;
        if (ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->isAtla())
        {
            compiler.addUniform("vec4", Echo::StringUtil::Format("%sViewport", getVariableName().c_str()));
            uvConvertCode = Echo::StringUtil::Format(" * fs_ubo.%sViewport.zw + fs_ubo.%sViewport.xy", getVariableName().c_str(), getVariableName().c_str());
        }

		if (ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->isAtla())
		{
			compiler.addUniform("vec4", Echo::StringUtil::Format("%sViewport", getVariableName().c_str()));
			uvConvertCode = Echo::StringUtil::Format(" * fs_ubo.%sViewport.zw + fs_ubo.%sViewport.xy", getVariableName().c_str(), getVariableName().c_str());
		}

        if (m_inputs[0])
        {
            compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Color = texture( %s, %s %s);\n", getVariableName().c_str(), getVariableName().c_str(), dynamic_cast<ShaderData*>(m_inputs[0].get())->getVariableName().c_str(), uvConvertCode.c_str()));
        }
        else
        {
            compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Color = texture( %s, v_UV %s);\n", getVariableName().c_str(), getVariableName().c_str(), uvConvertCode.c_str()));
        }


        if (ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->getType().getValue() == "General")
        {
            compiler.addCode(Echo::StringUtil::Format("\t%s_Color.rgb = SRgbToLinear(%s_Color.rgb);\n", getVariableName().c_str(), getVariableName().c_str()));
        }
        else
        {
            compiler.addMacro("ENABLE_VERTEX_NORMAL");

            compiler.addCode(Echo::StringUtil::Format("\t%s_Color.rgb = _NormalMapFun(%s_Color.rgb);\n", getVariableName().c_str(), getVariableName().c_str()));
        }

		return true;
	}

	bool TextureDataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_uniformConfig->isExport())
		{
            bool isAtla = ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->isAtla();

            uniformNames.emplace_back("Uniforms." + getVariableName());
            uniformValues.emplace_back(Echo::ResourcePath(m_textureSelect->getTexture(), isAtla ? ".png|.atla" : ".png"));

            if (ECHO_DOWN_CAST<Echo::ShaderNodeUniformTexture*>(m_uniformConfig)->isAtla())
            {
				uniformNames.emplace_back("Uniforms." + getVariableName() + "Viewport");
                uniformValues.emplace_back(Echo::Color(0.f, 0.f, 1.f, 1.f));
            }

			return true;
		}

		return false;
	}
}
