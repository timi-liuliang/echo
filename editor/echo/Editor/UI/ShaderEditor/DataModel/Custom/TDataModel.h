#pragma once

#include <QtCore/QObject>
#include <QtWidgets/QLineEdit>
#include <nodeeditor/NodeDataModel>
#include <iostream>
#include "ShaderDataModel.h"
#include "DataFloat.h"
#include "DataAny.h"
#include "ShaderData.h"
#include "engine/core/render/base/editor/shader/node/shader_node_glsl.h"

using QtNodes::PortType;
using QtNodes::PortIndex;
using QtNodes::NodeData;
using QtNodes::NodeDataType;
using QtNodes::NodeDataModel;
using QtNodes::NodeValidationState;

namespace DataFlowProgramming
{
    template<typename T>
    class TDataModel : public ShaderDataModel
    {
    public:
        TDataModel()
        {
            m_shaderNode = EchoNew(T);
			m_shaderNode->setInputDataTypesChangedCb([this](){ syncInputDataTypes(); });
        }

        virtual ~TDataModel() 
        {}

        // is caption visible
        bool captionVisible() const override 
        { 
            return true; 
        }

		// caption
		QString caption() const override 
		{ 
			return m_shaderNode->getCaption().c_str();
		}

		// name
		virtual QString name() const override 
		{ 
			return m_shaderNode->getName(); 
		}

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override
		{
			return m_shaderNode->generateCode(compiler);
		}

		// pressed event
		virtual bool onNodePressed() override
		{
			Studio::NodeTreePanel::instance()->onEditObject(m_shaderNode);
			return true;
		}

    public:
        // load|save
        virtual QJsonObject save() const override
        {
			QJsonObject nodeJson = NodeDataModel::save();
			saveShaderNode(nodeJson);

			return nodeJson;
        }

        virtual void restore(QJsonObject const& p) override
        {
            restoreShaderNode(p);
        }

    public:
        // when input changed
        void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override
        {

        }

        // widget
        QWidget* embeddedWidget() override 
        { 
            return nullptr; 
        }

	protected:
		// load|save
		void saveShaderNode(QJsonObject& p) const;
		void restoreShaderNode(QJsonObject const& p);

		// sync
		void syncInputDataTypes();

    protected:
        Echo::ShaderNode* m_shaderNode = nullptr;
    };

    template<typename T> void TDataModel<T>::saveShaderNode(QJsonObject& p) const
    {
		if (m_shaderNode)
		{
			Echo::PropertyInfos properties;
			Echo::Class::getPropertys(m_shaderNode->getClassName(), m_shaderNode, properties, 3, true);

			for (Echo::PropertyInfo* prop : properties)
			{
				Echo::Variant variant;
				Echo::Class::getPropertyValue(m_shaderNode, prop->m_name, variant);

				p[prop->m_name.c_str()] = variant.toString().c_str();
			}
		}
    }

    template<typename T> void TDataModel<T>::restoreShaderNode(QJsonObject const& p)
    {
		if (m_shaderNode)
		{
			Echo::PropertyInfos properties;
			Echo::Class::getPropertys(m_shaderNode->getClassName(), m_shaderNode, properties, 3, true);

			for (Echo::PropertyInfo* prop : properties)
			{
				QJsonValue v = p[prop->m_name.c_str()];
				if (!v.isUndefined())
				{
					Echo::String value = v.toString().toStdString().c_str();

					Echo::Variant variant;
					variant.fromString(prop->m_type, value);
					Echo::Class::setPropertyValue(m_shaderNode, prop->m_name, variant);
				}
			}
		}
    }

	template<typename T> void TDataModel<T>::syncInputDataTypes()
	{
		m_inputDataTypes.clear();
		Echo::ShaderNode::DataTypes inputTypes = m_shaderNode->getInputDataTypes();
		for (const Echo::ShaderNode::DataType& type : inputTypes)
		{
			m_inputDataTypes.push_back({ type.m_type, type.m_name });
		}

		m_inputs.resize(m_inputDataTypes.size());
	}
}
