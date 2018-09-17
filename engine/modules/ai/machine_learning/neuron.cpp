#include "neuron.h"
#include "neural_network.h"
#include "function/activation.h"

namespace Echo
{
	Neuron::Neuron()
		: m_bias( 0.f)
	{

	}

	Neuron::~Neuron()
	{

	}

	void Neuron::bindMethods()
	{
		BIND_METHOD(nn::sigmoid, "nn.sigmoid");

		CLASS_BIND_METHOD(Neuron, getBias, DEF_METHOD("getBias"));
		CLASS_BIND_METHOD(Neuron, setBias, DEF_METHOD("setBias"));

		CLASS_REGISTER_PROPERTY(Neuron, "Bias", Variant::Type::Real, "getBias", "setBias");
	}

	// get output
	Real Neuron::getOutput() const
	{
		Real iw = 0.f;
		for (const Input& input : m_inputs)
		{
			Real inputValue = m_network->getNeuron(input.m_neuron)->getOutput();
			iw += inputValue * input.m_weight;
		}

		return m_activationFunction ? (*m_activationFunction)(iw + m_bias) : 0.f;
	}
}