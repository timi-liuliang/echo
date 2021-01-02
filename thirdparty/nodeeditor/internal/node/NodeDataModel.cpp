#include "NodeDataModel.hpp"
#include "../style/StyleCollection.hpp"

using QtNodes::NodeDataModel;
using QtNodes::NodeStyle;

namespace QtNodes
{
	NodeDataModel::NodeDataModel()
		: m_nodeStyle(StyleCollection::nodeStyle())
	{
		// Derived classes can initialize specific style here
	}

	QJsonObject NodeDataModel::save() const
	{
		QJsonObject modelJson;

		modelJson["name"] = name();

		return modelJson;
	}

	NodeStyle const& NodeDataModel::nodeStyle() const
	{
		return m_nodeStyle;
	}

	void NodeDataModel::setNodeStyle(NodeStyle const& style)
	{
		m_nodeStyle = style;
	}

}
