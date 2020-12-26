#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include "propertyeditor/QVector4Editor.h"
#include <iostream>
#include "DataFloat.h"
#include "ShaderUniformDataModel.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    /// The model dictates the number of inputs and outputs for the Node.
    /// In this example it has no logic.
    class Vector4DataModel : public ShaderUniformDataModel
    {
      Q_OBJECT

    public:
        Vector4DataModel();
        virtual ~Vector4DataModel() {}

        virtual QString name() const override { return QStringLiteral("Vector4"); }

		// generate code
        virtual bool generateCode(ShaderCompiler& compiler) override;

		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

    public:
        // load|save
        QJsonObject save() const override;
        void restore(QJsonObject const &p) override;

    public:
        // set in data
        void setInData(std::shared_ptr<NodeData>, int) override { }

        // get embedded widget
        QWidget* embeddedWidget() override { return m_vector4Editor; }

	private:
		// update outputs variable name
		void updateOutputDataVariableName();

		// variable changed
		void onVariableNameChanged();

    private Q_SLOTS:
        // on value changed
        void onTextEdited();

    private:
        QT_UI::QVector4Editor*   m_vector4Editor;
    };
}

