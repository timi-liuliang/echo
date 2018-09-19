#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/math/matrix.h"

namespace Echo
{
	/* thanks very much to this article
	 * https://causeyourestuck.io/2017/06/12/neural-network-scratch-theory/
	 */
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
		void train(const RealVector& inputVector, const RealVector& expectedOutput);

		// compute output
		RealVector computeOutput(const RealVector& inputVector);

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
		void learn(const RealVector& expectedOutput);

	protected:
		ActivationFunction			m_activationFunction;
		LossFunction				m_lossFunction;
		Real						m_learningRate;			// learning speed
		vector<Matrix>				m_layerValues;
		vector<Matrix>				m_weights;
		vector<Matrix>				m_bias;
	};
}