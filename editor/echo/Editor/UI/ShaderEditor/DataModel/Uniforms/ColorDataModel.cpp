#include "ColorDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector3.h"
#include "Engine/core/util/StringUtil.h"
#include <QMenu>

namespace DataFlowProgramming
{
    ColorDataModel::ColorDataModel()
        : ShaderUniformDataModel()
    {
        m_outputs.resize(5);

        m_colorSelect = new QT_UI::QColorSelect();
        m_colorSelect->setFixedSize(128, 128);
        m_colorSelect->setDrawText(false);

        QObject::connect(m_colorSelect, SIGNAL(Signal_ColorChanged()), this, SLOT(onColorEdited()));
		QObject::connect(m_setAsParameter, SIGNAL(triggered()), this, SLOT(onSetAsParameter()));
		QObject::connect(m_setAsConstant, SIGNAL(triggered()), this, SLOT(onSetAsConstant()));

        updateOutputDataVariableName();
    }

    QJsonObject ColorDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        ShaderUniformDataModel::saveUniformConfig(modelJson);

        modelJson["color"] = Echo::StringUtil::ToString(m_colorSelect->GetColor()).c_str();

        return modelJson;
    }

    void ColorDataModel::restore(QJsonObject const &p)
    {
        ShaderUniformDataModel::restoreUniformConfig(p);

        QJsonValue v = p["color"];
        if (!v.isUndefined())
        {
            Echo::String colorStr = v.toString().toStdString().c_str();
            m_colorSelect->SetColor(Echo::StringUtil::ParseColor(colorStr));
        }
    }

    void ColorDataModel::onColorEdited()
    {
        Q_EMIT dataUpdated(0);
        Q_EMIT dataUpdated(1);
        Q_EMIT dataUpdated(2);
        Q_EMIT dataUpdated(3);
        Q_EMIT dataUpdated(4);
    }

	void ColorDataModel::onSetAsParameter()
    {
		m_isParameter = true;
		updateOutputDataVariableName();

        onColorEdited();
	}

	void ColorDataModel::onSetAsConstant()
    {
		m_isParameter = false;
		updateOutputDataVariableName();

        onColorEdited();
	}

    void ColorDataModel::updateOutputDataVariableName()
    {
        Echo::String variableName = getVariableName();

		m_outputs[0] = std::make_shared<DataVector3>(this, "rgb");
		m_outputs[1] = std::make_shared<DataFloat>(this, "r");
		m_outputs[2] = std::make_shared<DataFloat>(this, "g");
		m_outputs[3] = std::make_shared<DataFloat>(this, "b");
		m_outputs[4] = std::make_shared<DataFloat>(this, "a");

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value.rgb", variableName.c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s_Value.r", variableName.c_str()));
		m_outputs[2]->setVariableName(Echo::StringUtil::Format("%s_Value.g", variableName.c_str()));
		m_outputs[3]->setVariableName(Echo::StringUtil::Format("%s_Value.b", variableName.c_str()));
		m_outputs[4]->setVariableName(Echo::StringUtil::Format("%s_Value.a", variableName.c_str()));
    }

	bool ColorDataModel::generateCode(ShaderCompiler& compiler)
	{
		if (m_isParameter)
		{
			compiler.addUniform("vec4", getVariableName().c_str());

            compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(SRgbToLinear(fs_ubo.%s.rgb), fs_ubo.%s.a);\n", getVariableName().c_str(), getVariableName().c_str(), getVariableName().c_str()));
		}
		else
		{
			Echo::Color color = m_colorSelect->GetColor();
            color.toLinear();

			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), color.r, color.g, color.b, color.a));
		}

		return true;
	}
}
