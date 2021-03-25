#pragma once

#include "engine/core/scene/node.h"
#include "engine/modules/pcg/data/pcg_data.h"
#include "engine/modules/pcg/connect/pcg_connect_point.h"

namespace Echo
{
	class PCGFlowGraph;
	class PCGNode : public Object
	{
		ECHO_CLASS(PCGNode, Object);

	public:
		PCGNode();
		virtual ~PCGNode();

		// Name
		String getName() const { return m_name; }
		void setName(const String& name) { m_name = name; }

		// Graph
		void setGraph(PCGFlowGraph* graph) { m_graph = graph; }
		PCGFlowGraph* getGraph() { return m_graph; }

		// Output
		const std::vector<PCGConnectPoint*>& getInputs() { return m_inputs; }
		const std::vector<PCGConnectPoint*>& getOutputs() { return m_outputs; }

		// Dependent Inputs
		virtual std::vector<PCGConnectPoint*> getDependentInputs() { return m_inputs; }

		// pos
		void setPosition(const Vector2& position) { m_position = position; }
		const Vector2& getPosition() { return m_position; }

		// is dirty
		bool isDirty() { return m_dirtyFlag; }

		// check
		bool check() { return true; }

		// calculate
		virtual void run();

	protected:
		String								m_name;
		PCGFlowGraph*						m_graph = nullptr;
		std::vector<PCGConnectPoint*>		m_inputs;
		std::vector<PCGConnectPoint*>		m_outputs;
		bool								m_dirtyFlag = true;
		Vector2								m_position;
	};
}