#pragma once

#include <QColor>
#include <QList>
#include <QString>

namespace Studio
{
	struct LineNode
	{
		float beginTime;
		float duration;
		QColor color;
		QString name;
		QString text; 
		LineNode()
			: beginTime(0.f)
			, duration(0.f)
		{}
	};

	class LineItem
	{
	public:
		LineItem() {}
		LineItem(const QString& name)
			: m_name(name)
			, m_checkState(Qt::Unchecked)
			, m_layerType(0)
		{}
		~LineItem() {}
		void setName(const QString& name) { m_name = name; }
		QString name() const { return m_name; }
		void setCheckState(Qt::CheckState state) { m_checkState = state; }
		void setLayerType(int type){ m_layerType = type; }
		int getLayerType() const { return m_layerType; }
		Qt::CheckState checkState() const { return m_checkState; }
		void addNode(const LineNode& node) { m_nodes.push_back(node); }
		LineNode node(int index) const { return m_nodes.at(index); }
		int size() const { return m_nodes.size(); }
	private:
		QString m_name;
		Qt::CheckState m_checkState;
		QList<LineNode> m_nodes;
		int m_layerType;
	};

} // namespace Studio
