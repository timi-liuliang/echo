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

		// train
		void train(vector<Real>::type& input, vector<Real>::type& output);

		// neuron
		Neuron* getNeuron(i32 layer, i32 idx);

		// learning rate
		Real getLearningRate() const { return m_learningRate; }
		void setLearningRate(Real rate) { m_learningRate = rate; }

		// reset
		void reset();

	protected:
		// organize by node tree structure
		void organzieStructureBaseOnNodeTree();

	protected:
		LossFunction				m_lossFunction;
		Real						m_learningRate;		// learning speed
	};
}