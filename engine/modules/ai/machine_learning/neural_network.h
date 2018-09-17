#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Neuron;
	class NeuralNetwork : public Node
	{
		ECHO_CLASS(NeuralNetwork, Node)

	public:
		typedef Real(*LossFunction)(Real);

	public:
		NeuralNetwork();

		// neuron
		Neuron* getNeuron(ui32 idx);

	protected:
		vector<Neuron*>::type		m_neurons;
		LossFunction				m_lossFunction;
	};
}