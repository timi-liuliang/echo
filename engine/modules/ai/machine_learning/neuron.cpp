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

		CLASS_BIND_METHOD(Neuron, getBias);
		CLASS_BIND_METHOD(Neuron, setBias);

		CLASS_REGISTER_PROPERTY(Neuron, "Bias", Variant::Type::Real, getBias, setBias);
	}
}