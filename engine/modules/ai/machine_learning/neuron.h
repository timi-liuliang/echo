#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class NeuralNetwork;
	class Neuron : public Node
	{
		ECHO_CLASS(Neuron, Node)

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

	protected:
		NeuralNetwork*			m_network;				// the neural net work this neuron belong to
		i32						m_layer;
		vector<Real>::type		m_weights;
		Real					m_bias;
		Real					m_value;
	};
}