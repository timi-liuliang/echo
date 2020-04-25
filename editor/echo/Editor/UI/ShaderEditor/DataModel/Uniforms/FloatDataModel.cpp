#include "FloatDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace DataFlowProgramming
{
    FloatDataModel::FloatDataModel()
      : ShaderUniformDataModel()
      , m_lineEdit(new QLineEdit())
    {
        m_uniformConfig = EchoNew(Echo::ShaderUniform);

        m_lineEdit->setValidator(new QDoubleValidator());
        m_lineEdit->setMaximumSize(QSize(m_lineEdit->sizeHint().width() * 0.4f, m_lineEdit->sizeHint().height()));
        m_lineEdit->setText("0.0");

        QObject::connect(m_lineEdit, SIGNAL(editingFinished()), this, SLOT(onTextEdited()));

        m_outputs.resize(1);
        m_outputs[0] = std::make_shared<DataFloat>(this, "float");
        m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s", getVariableName().c_str()));
    }

    QJsonObject FloatDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        modelJson["number"] = m_lineEdit->text().toStdString().c_str();

        return modelJson;
    }

    void FloatDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            m_lineEdit->setText(v.toString());
        }
    }

    void FloatDataModel::onTextEdited()
    {
        float number = Echo::StringUtil::ParseFloat(m_lineEdit->text().toStdString().c_str());
        m_lineEdit->setText(Echo::StringUtil::ToString(number).c_str());

        Q_EMIT dataUpdated(0);
    }

    bool FloatDataModel::generateCode(ShaderCompiler& compiler)
    {
		float number = Echo::StringUtil::ParseFloat(m_lineEdit->text().toStdString().c_str());
		compiler.addCode(Echo::StringUtil::Format("\tfloat %s = %f;\n", getVariableName().c_str(), number));

		return true;
    }

	bool FloatDataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		return false;
	}
}
