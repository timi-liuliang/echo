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
	Real Neuron::getOutput()
	{
		//if (m_layer != 0)
		//{
		//	Real iw = 0.f;
		//	for (size_t i = 0; i < m_weights.size(); i++)
		//	{
		//		Real inputValue = m_network->getNeuron(m_layer - 1, i)->getOutput();
		//		iw += inputValue * m_weights[i];
		//	}

		//	m_value = m_activationFunction ? (*m_activationFunction)(iw + m_bias) : 0.f;
		//}

		return m_value;
	}
}