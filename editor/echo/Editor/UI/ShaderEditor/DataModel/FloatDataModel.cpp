#include "FloatDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace DataFlowProgramming
{
    FloatDataModel::FloatDataModel()
      : _lineEdit(new QLineEdit())
    {
      _lineEdit->setValidator(new QDoubleValidator());

      _lineEdit->setMaximumSize(_lineEdit->sizeHint());

      connect(_lineEdit, &QLineEdit::textChanged, this, &FloatDataModel::onTextEdited);

      _lineEdit->setText("0.0");
    }


    QJsonObject FloatDataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

        return modelJson;
    }

    void FloatDataModel::restore(QJsonObject const &p)
    {
      QJsonValue v = p["number"];

      if (!v.isUndefined())
      {
        QString strNum = v.toString();

        bool   ok;
        double d = strNum.toDouble(&ok);
        if (ok)
        {
          //_number = std::make_shared<DataFloat>("float");
          //_lineEdit->setText(strNum);
        }
      }
    }


    unsigned int FloatDataModel::nPorts(PortType portType) const
    {
      unsigned int result = 1;

      switch (portType)
      {
        case PortType::In:
          result = 0;
          break;

        case PortType::Out:
          result = 1;

        default:
          break;
      }

      return result;
    }


    void FloatDataModel::onTextEdited(QString const &string)
    {
      Q_UNUSED(string);

      bool ok = false;

      double number = _lineEdit->text().toDouble(&ok);

      if (ok)
      {
        //_number = std::make_shared<DataFloat>(number);

        Q_EMIT dataUpdated(0);
      }
      else
      {
        Q_EMIT dataInvalidated(0);
      }
    }


    NodeDataType FloatDataModel::dataType(PortType, PortIndex) const
    {
        return NodeDataType {"float", "float"};
    }


    std::shared_ptr<NodeData> FloatDataModel::outData(PortIndex)
    {
      return _number;
    }

}
