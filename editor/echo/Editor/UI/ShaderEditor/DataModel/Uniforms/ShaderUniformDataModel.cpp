#include "ShaderUniformDataModel.h"
#include "NodeTreePanel.h"

namespace DataFlowProgramming
{
	ShaderUniformDataModel::ShaderUniformDataModel()
		: ShaderDataModel()
	{
		m_setAsParameter = new QAction("Switch to Parameter");
		m_setAsConstant = new QAction("Switch to Constant");
	}

	QString ShaderUniformDataModel::caption() const
	{
		return m_uniformConfig->getVariableName().c_str();
	}

	Echo::String ShaderUniformDataModel::getVariableName() const
	{ 
		if (m_uniformConfig->isExport() && !m_uniformConfig->getVariableName().empty())
			return m_uniformConfig->getVariableName();
		else
			return getDefaultVariableName();
	}

	void ShaderUniformDataModel::saveUniformConfig(QJsonObject& p) const
	{
		p["Export"] = Echo::StringUtil::ToString(m_uniformConfig->isExport()).c_str();
		p["variableName"] = getVariableName().c_str();
	}

	void ShaderUniformDataModel::restoreUniformConfig(QJsonObject const &p)
	{
		QJsonValue v = p["Export"];
		if (!v.isUndefined())
		{
			Echo::String variableName = v.toString().toStdString().c_str();
			m_uniformConfig->setExport(Echo::StringUtil::ParseBool(variableName));
		}

		v = p["variableName"];
		if (!v.isUndefined())
		{
			Echo::String variableName = v.toString().toStdString().c_str();
			m_uniformConfig->setVariableName(variableName);
		}
	}

	bool ShaderUniformDataModel::onNodePressed()
	{
		if (m_uniformConfig->getVariableName().empty())
			m_uniformConfig->setVariableName(getDefaultVariableName());

		Studio::NodeTreePanel::instance()->onEditObject(m_uniformConfig);

		return true;
	}

	void ShaderUniformDataModel::showMenu(const QPointF& pos)
	{
		if (!m_menu)
		{
			m_menu = new QMenu();
		}

		m_menu->clear();
		m_menu->addAction(m_uniformConfig->isExport() ? m_setAsConstant : m_setAsParameter);

		m_menu->exec(QCursor::pos());
	}
}