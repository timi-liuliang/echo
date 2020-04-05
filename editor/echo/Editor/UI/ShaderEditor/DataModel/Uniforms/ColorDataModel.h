#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "DataFloat.h"
#include "QColorSelect.h"
#include "ShaderDataModel.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    class ColorDataModel : public ShaderDataModel
    {
      Q_OBJECT

    public:
        ColorDataModel();
        virtual ~ColorDataModel() {}

        // caption
        virtual QString caption() const override;
        bool captionVisible() const override { return m_isParameter; }

        virtual QString name() const override { return QStringLiteral("Color"); }

		// show menu
        virtual void showMenu(const QPointF& pos) override;

        // generate code
        virtual bool generateCode(ShaderCompiler& compiler) override;
        
    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        // get port type
        unsigned int nPorts(PortType portType) const override;

        // get data type
        NodeDataType dataType(PortType portType, PortIndex portIndex) const override;

        std::shared_ptr<NodeData> outData(PortIndex port) override;

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

		// slot
        virtual void onDoubleClicked();

    private:
        QMenu*                              m_menu = nullptr;
        QAction*                            m_setAsParameter = nullptr;
		QAction*                            m_setAsConstant = nullptr;
        QT_UI::QColorSelect*                m_colorSelect = nullptr;
        vector<std::shared_ptr<ShaderData>> m_outputs;
    };
}

