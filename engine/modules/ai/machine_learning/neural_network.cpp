#include "neural_network.h"

namespace Echo
{
	NeuralNetwork::NeuralNetwork()
		:m_lossFunction(nullptr)
	{

	}

	void NeuralNetwork::bindMethods()
	{
	}

	// neuron
	Neuron* NeuralNetwork::getNeuron(ui32 idx)
	{
		return m_neurons[idx];
	}
}