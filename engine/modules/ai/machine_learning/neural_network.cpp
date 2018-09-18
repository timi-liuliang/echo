#include "neural_network.h"

namespace Echo
{
	NeuralNetwork::NeuralNetwork()
		: m_lossFunction(nullptr)
		, m_learningRate(0.f)
	{

	}

	void NeuralNetwork::bindMethods()
	{
		CLASS_BIND_METHOD(NeuralNetwork, getLearningRate, DEF_METHOD("getLearningRate"));
		CLASS_BIND_METHOD(NeuralNetwork, setLearningRate, DEF_METHOD("setLearningRate"));

		CLASS_REGISTER_PROPERTY(NeuralNetwork, "LearningRate", Variant::Type::Real, "getLearningRate", "setLearningRate");
	}

	// neuron
	Neuron* NeuralNetwork::getNeuron(ui32 idx)
	{
		return nullptr;
	}
}