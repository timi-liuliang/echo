#include "neural_network.h"

namespace Echo
{
	void NeuralNetwork::bindMethods()
	{

	}

	// neuron
	Neuron* NeuralNetwork::getNeuron(ui32 idx)
	{
		return m_neurons[idx];
	}
}