#pragma once

#include "engine/core/scene/node.h"

namespace Echo
{
	class Neuron;
	class NeuralNetwork : public Node
	{
		ECHO_CLASS(NeuralNetwork, Node)

	public:
		typedef Real(*LossFunction)(Real);

	public:
		NeuralNetwork();

		// neuron
		Neuron* getNeuron(ui32 idx);

		// learning rate
		Real getLearningRate() const { return m_learningRate; }
		void setLearningRate(Real rate) { m_learningRate = rate; }

	protected:
		LossFunction				m_lossFunction;
		Real						m_learningRate;		// learning speed
	};
}