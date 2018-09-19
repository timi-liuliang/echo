#pragma once

#include "engine/core/scene/node.h"
#include "math/matrix.h"

namespace Echo
{
	class Neuron;
	class NeuralNetwork : public Node
	{
		ECHO_CLASS(NeuralNetwork, Node)

	public:
		typedef Real(*ActivationFunction)(Real);
		typedef Real(*LossFunction)(Real);

	public:
		NeuralNetwork();

		// train
		void train(const nn::DoubleVector& inputVector, const nn::DoubleVector& expectedOutput);

		// compute output
		nn::DoubleVector computeOutput(const nn::DoubleVector& inputVector);

		// neuron
		Neuron* getNeuron(i32 layer, i32 idx);

		// learning rate
		Real getLearningRate() const { return m_learningRate; }
		void setLearningRate(Real rate) { m_learningRate = rate; }

		// activation function
		void setActivationFunction(ActivationFunction fun) { m_activationFunction = fun; }
		ActivationFunction getActivationFunction() { return m_activationFunction; }

		// reset
		void reset();

	protected:
		// organize by node tree structure
		void organzieStructureBaseOnNodeTree();

		// learn
		void learn(const nn::DoubleVector& expectedOutput);

	protected:
		ActivationFunction			m_activationFunction;
		LossFunction				m_lossFunction;
		Real						m_learningRate;			// learning speed
		vector<nn::Matrix>			m_layerValues;
		vector<nn::Matrix>			m_weights;
		vector<nn::Matrix>			m_bias;
	};
}