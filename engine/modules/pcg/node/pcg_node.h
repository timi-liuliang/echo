#pragma once

#include "engine/core/scene/node.h"
#include "engine/modules/pcg/data/pcg_data.h"
#include "engine/modules/pcg/connect/pcg_connect_point.h"

namespace Echo
{
	class PCGNode : public Object
	{
		ECHO_CLASS(PCGNode, Object);

	public:
		// Type
		virtual String getType() const { return "UnKnown"; }

		// Output
		const std::vector<std::shared_ptr<PCGConnectPoint>>& GetInputs() { return Inputs; }
		const std::vector<std::shared_ptr<PCGConnectPoint>>& GetOutputs() { return Outputs; }

		// Dependent Inputs
		virtual std::vector<std::shared_ptr<PCGConnectPoint>> GetDependentInputs() { return Inputs; }

		// pos
		void setPosition(const Vector2& position) { m_position = position; }
		const Vector2& getPosition() { return m_position; }

		// is dirty
		bool isDirty() { return m_dirtyFlag; }

		// calculate
		virtual void run();

	protected:
		std::vector<std::shared_ptr<PCGConnectPoint>>	Inputs;
		std::vector<std::shared_ptr<PCGConnectPoint>>	Outputs;
		bool											m_dirtyFlag = true;
		Vector2											m_position;
	};
}