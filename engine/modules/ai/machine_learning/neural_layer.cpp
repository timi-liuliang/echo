#include "neural_layer.h"
#include "neural_network.h"
#include "function/activation.h"

namespace Echo
{
	NeuralLayer::NeuralLayer()
	{

	}

	NeuralLayer::~NeuralLayer()
	{

	}

	void NeuralLayer::bindMethods()
	{
	}

	// get neural number
	i32 NeuralLayer::getNeuralNumber()
	{
		i32 neuronNumber = 0;
		for (Node* child : getChildren())
		{
			if (child->getClassName() == "Neuron")
				neuronNumber++;
		}

		return neuronNumber;
	}
}