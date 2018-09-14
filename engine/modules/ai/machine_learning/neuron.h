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

		// input
		struct Input
		{
			ui32	m_neuron;		// the id of input neuron
			Real	m_weight;
		};
		typedef vector<Input>::type	Inputs;

	public:
		Neuron();
		virtual ~Neuron();

		// get output
		Real getOutput() const;

		// bias
		void setBias(Real bias) { m_bias = bias; }
		Real getBias() const { return m_bias; }

		// activation function
		void setActivationFunction(ActivationFunction fun) { m_activationFunction = fun; }
		ActivationFunction getActivationFunction() { return m_activationFunction; }

	protected:
		NeuralNetwork*			m_network;				// the neural net work this neuron belong to
		Inputs					m_inputs;
		Real					m_bias;
		ActivationFunction		m_activationFunction;
	};
}