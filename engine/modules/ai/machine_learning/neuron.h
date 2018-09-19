#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class NeuralNetwork;
	class Neuron : public Node
	{
		ECHO_CLASS(Neuron, Node)

	public:
		typedef Real(*ActivationFunction)(Real);

	public:
		Neuron();
		virtual ~Neuron();

		// get output
		Real getOutput();

		// layer
		void setLayer(i32 layer) { m_layer = layer; }

		// value
		void setValue(Real value) { m_value = value; }

		// bias
		void setBias(Real bias) { m_bias = bias; }
		Real getBias() const { return m_bias; }

		// activation function
		void setActivationFunction(ActivationFunction fun) { m_activationFunction = fun; }
		ActivationFunction getActivationFunction() { return m_activationFunction; }

	protected:
		NeuralNetwork*			m_network;				// the neural net work this neuron belong to
		i32						m_layer;
		vector<Real>::type		m_weights;
		Real					m_bias;
		Real					m_value;
		ActivationFunction		m_activationFunction;
	};
}