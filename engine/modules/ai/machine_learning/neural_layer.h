#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class NeuralNetwork;
	class NeuralLayer : public Node
	{
		ECHO_CLASS(NeuralLayer, Node)

	public:
		NeuralLayer();
		virtual ~NeuralLayer();

		// get neural number
		i32 getNeuralNumber();

	protected:
	};
}