#include "Vector3DataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"

namespace ShaderEditor
{
    Vector3DataModel::Vector3DataModel()
      : _lineEdit(new QLineEdit())
    {
      _lineEdit->setValidator(new QDoubleValidator());

      _lineEdit->setMaximumSize(_lineEdit->sizeHint());

      connect(_lineEdit, &QLineEdit::textChanged, this, &Vector3DataModel::onTextEdited);

      _lineEdit->setText("0.0");
    }


    QJsonObject Vector3DataModel::save() const
    {
      QJsonObject modelJson = NodeDataModel::save();

      if (_number)
        modelJson["number"] = QString::number(_number->number());

      return modelJson;
    }


    void Vector3DataModel::restore(QJsonObject const &p)
    {
      QJsonValue v = p["number"];

      if (!v.isUndefined())
      {
        QString strNum = v.toString();

        bool   ok;
        double d = strNum.toDouble(&ok);
        if (ok)
        {
          _number = std::make_shared<DataFloat>(d);
          _lineEdit->setText(strNum);
        }
      }
    }


    unsigned int Vector3DataModel::nPorts(PortType portType) const
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


    void Vector3DataModel::onTextEdited(QString const &string)
    {
      Q_UNUSED(string);

      bool ok = false;

      double number = _lineEdit->text().toDouble(&ok);

      if (ok)
      {
        _number = std::make_shared<DataFloat>(number);

        Q_EMIT dataUpdated(0);
      }
      else
      {
        Q_EMIT dataInvalidated(0);
      }
    }


    NodeDataType Vector3DataModel::dataType(PortType, PortIndex) const
    {
        return NodeDataType {"vec3", "vec3"};
    }


    std::shared_ptr<NodeData> Vector3DataModel::outData(PortIndex)
    {
      return _number;
    }

}
