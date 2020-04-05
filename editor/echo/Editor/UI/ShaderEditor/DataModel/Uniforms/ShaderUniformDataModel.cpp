#include "ShaderUniformDataModel.h"
#include "Data/DataAny.h"
#include "NodeTreePanel.h"

namespace DataFlowProgramming
{
	ShaderUniformDataModel::ShaderUniformDataModel()
		: ShaderDataModel()
	{
		m_uniformConfig = EchoNew(Echo::ShaderUniformConfig);

		m_setAsParameter = new QAction("Switch to Parameter");
		m_setAsConstant = new QAction("Switch to Constant");
	}

	QString ShaderUniformDataModel::caption() const
	{
		return m_uniformConfig->getVariableName().c_str();
	}

	Echo::String ShaderUniformDataModel::getVariableName() const
	{ 
		if (m_isParameter && !m_uniformConfig->getVariableName().empty())
			return m_uniformConfig->getVariableName();
		else
			return getDefaultVariableName();
	}

	void ShaderUniformDataModel::saveUniformConfig(QJsonObject const& p) const
	{
		p["isParameter"] = Echo::StringUtil::ToString(m_isParameter).c_str();
		p["variableName"] = getVariableName().c_str();
	}

	void ShaderUniformDataModel::restoreUniformConfig(QJsonObject const& p)
	{
		QJsonValue v = p["isParameter"];
		if (!v.isUndefined())
		{
			Echo::String variableName = v.toString().toStdString().c_str();
			m_isParameter = Echo::StringUtil::ParseBool(variableName);
		}

		v = p["variableName"];
		if (!v.isUndefined())
		{
			Echo::String variableName = v.toString().toStdString().c_str();
			m_uniformConfig->setVariableName(variableName);
		}
	}

	void ShaderUniformDataModel::onDoubleClicked()
	{
		if (m_isParameter)
		{
			if (m_uniformConfig->getVariableName().empty())
				m_uniformConfig->setVariableName(getDefaultVariableName());

			Studio::NodeTreePanel::instance()->onEditObject(m_uniformConfig);
		}
	}

	void ShaderUniformDataModel::showMenu(const QPointF& pos)
	{
		if (!m_menu)
		{
			m_menu = new QMenu();
		}

		m_menu->clear();
		m_menu->addAction(m_isParameter ? m_setAsConstant : m_setAsParameter);

		m_menu->exec(QCursor::pos());
	}
}