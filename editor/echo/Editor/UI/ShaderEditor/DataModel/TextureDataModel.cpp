#include "TextureDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"

namespace DataFlowProgramming
{
    TextureDataModel::TextureDataModel()
    {
        m_textureSelect = new QT_UI::QTextureSelect();
        m_textureSelect->setFixedSize(155, 155);

       QObject::connect(m_textureSelect, SIGNAL(Signal_TextureChagned()), this, SLOT(onTextureEdited()));

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

        modelJson["texture"] = m_textureSelect->getTexture().c_str();

        return modelJson;
    }

    void TextureDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["texture"];
        if (!v.isUndefined())
        {
            Echo::String texturePath = v.toString().toStdString().c_str();
            m_textureSelect->setTexture(texturePath);
        }
    }

    unsigned int TextureDataModel::nPorts(PortType portType) const
    {
		switch (portType)
		{
		case PortType::In:    return 0;
		case PortType::Out:   return m_outputs.size();
		default:              return 0;
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

    NodeDataType TextureDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        if(portType==PortType::Out)
        {
            if(portIndex==0)      return NodeDataType {"vec3", "rgb"};
            else if(portIndex==1) return NodeDataType {"float", "r"};
            else if(portIndex==2) return NodeDataType {"float", "g"};
            else if(portIndex==3) return NodeDataType {"float", "b"};
            else if(portIndex==4) return NodeDataType {"float", "a"};
        }
        
        return NodeDataType {"unknown", "Unknown"};
    }

    std::shared_ptr<NodeData> TextureDataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

    bool TextureDataModel::generateCode(ShaderCompiler& compiler)
    {
        compiler.addTextureUniform(getVariableName());

        compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Color = texture( %s, v_UV);\n", getVariableName().c_str(), getVariableName().c_str()));

        return true;
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
}
