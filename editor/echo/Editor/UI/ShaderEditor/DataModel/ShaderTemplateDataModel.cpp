#include "ShaderTemplateDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DecimalData.h"

namespace ShaderEditor
{
    ShaderTemplateDataModel::ShaderTemplateDataModel()
     : m_lineEdit(new QLineEdit())
    {
        m_lineEdit->setValidator(new QDoubleValidator());
        m_lineEdit->setMaximumSize(m_lineEdit->sizeHint());

        QObject::connect(m_lineEdit, &QLineEdit::textChanged, this, &ShaderTemplateDataModel::onTextEdited);

        m_lineEdit->setText("0.0");
    }

    QJsonObject ShaderTemplateDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

        if (m_number)
            modelJson["number"] = QString::number(m_number->number());

        return modelJson;
    }

    void ShaderTemplateDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];

        if (!v.isUndefined())
        {
            QString strNum = v.toString();

            bool   ok;
            double d = strNum.toDouble(&ok);
            if (ok)
            {
                m_number = std::make_shared<DecimalData>(d);
                m_lineEdit->setText(strNum);
            }
        }
    }

    unsigned int ShaderTemplateDataModel::nPorts(PortType portType) const
    {
        unsigned int result = 1;

        switch (portType)
        {
            case PortType::In: result = 0; break;
            case PortType::Out:result = 1; break;
            default:                       break;
        }

      return result;
    }

    void ShaderTemplateDataModel:: onTextEdited(QString const &string)
    {
        Q_UNUSED(string);

        bool ok = false;

        double number = m_lineEdit->text().toDouble(&ok);
        if (ok)
        {
            m_number = std::make_shared<DecimalData>(number);

            Q_EMIT dataUpdated(0);
        }
        else
        {
            Q_EMIT dataInvalidated(0);
        }
    }

    NodeDataType ShaderTemplateDataModel::dataType(PortType, PortIndex) const
    {
        return NodeDataType {"decimal", "Decimal"};
    }

    std::shared_ptr<NodeData> ShaderTemplateDataModel::outData(PortIndex)
    {
        return m_number;
    }
}
