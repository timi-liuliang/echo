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

		// layer
		void setLayer(i32 layer) { m_layer = layer; }

		// value
		Real getValue() const { return m_value; }
		void setValue(Real value) { m_value = value; }

		// weights
		const RealVector& getWeights() const { return m_weights; }
		void setWeights(const RealVector& weights) { m_weights = weights; }

		// bias
		void setBias(Real bias) { m_bias = bias; }
		Real getBias() const { return m_bias; }

	protected:
		NeuralNetwork*			m_network;				// the neural net work this neuron belong to
		i32						m_layer;
		RealVector				m_weights;
		Real					m_bias;
		Real					m_value;
	};
}