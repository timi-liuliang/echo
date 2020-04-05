#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "DataFloat.h"
#include "QColorSelect.h"
#include "ShaderUniformDataModel.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    class ColorDataModel : public ShaderUniformDataModel
    {
      Q_OBJECT

    public:
        ColorDataModel();
        virtual ~ColorDataModel() {}

        virtual QString name() const override { return QStringLiteral("Color"); }

        // generate code
        virtual bool generateCode(ShaderCompiler& compiler) override;
        
    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        void setInData(std::shared_ptr<NodeData>, int) override { }

        // get embedded widget
        QWidget* embeddedWidget() override { return m_colorSelect; }

    private:
        // update outputs variable name
        void updateOutputDataVariableName();

    private Q_SLOTS:
        // on value changed
        void onColorEdited();

        // switch between parameter with constant
        void onSetAsParameter();
        void onSetAsConstant();

    private:
        QT_UI::QColorSelect*                m_colorSelect = nullptr;
    };
}

