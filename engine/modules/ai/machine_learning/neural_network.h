#pragma once

#include "engine/core/scene/node.h"
#include "engine/core/math/matrix.h"

namespace Echo
{
	/* thanks very much to this article
	 * https://causeyourestuck.io/2017/06/12/neural-network-scratch-theory/
	 */
	class Neuron;
	class NeuralLayer;
	class NeuralNetwork : public Node
	{
		ECHO_CLASS(NeuralNetwork, Node)

	public:
		typedef Real(*MatrixFunction)(Real);
		typedef Matrix(*LossFunction)(const Matrix&, const Matrix&);

	public:
		NeuralNetwork();

		// train
		void train(const Matrix& inputVector, const Matrix& expectedOutput);

		// compute output
		Matrix computeOutput(const Matrix& inputVector);

		// layer
		i32 getLayerNumber();
		NeuralLayer* getLayer(i32 layer);

		// neuron
		i32 getNeuronNum(i32 layer);
		Neuron* getNeuron(i32 layer, i32 idx);

		// learning rate
		Real getLearningRate() const { return m_learningRate; }
		void setLearningRate(Real rate) { m_learningRate = rate; }

		// activation function
		void setActivationFunction(MatrixFunction fun) { m_activationFunction = fun; }
		MatrixFunction getActivationFunction() { return m_activationFunction; }

		// activation function prime
		void setActivationFunctionPrime(MatrixFunction fun) { m_activationFunctionPrime = fun; }
		MatrixFunction getActivationFunctionPrime() { return m_activationFunctionPrime; }

		// loss function prime
		void setLossFunctionPrime(LossFunction fun) { m_lossFunctionPrime = fun; }
		LossFunction getLossFunctionPrime() { return m_lossFunctionPrime; }

		// reset
		void reset();

	protected:
		// organize by node tree structure
		void organzieStructureBaseOnNodeTree();

		// learn
		void learn(const Matrix& expectedOutput);

		// compute output by layer
		Matrix computeLayerOutput(i32 layer, MatrixFunction fun);

	protected:
		bool						m_isInit;
		MatrixFunction				m_activationFunction;
		MatrixFunction				m_activationFunctionPrime;
		LossFunction				m_lossFunctionPrime;
		Real						m_learningRate;			// learning speed
		vector<Matrix>::type		m_layerValues;
		vector<Matrix>::type		m_weights;
		vector<Matrix>::type		m_dJdWeights;			// partial derivative of loss function with with respect to weights
		vector<Matrix>::type		m_bias;
		vector<Matrix>::type		m_dJdBias;				// partial derivative of loss function with with respect to bias
	};
}