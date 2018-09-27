#include "function/activation.h"
#include "function/loss.h"
#include "neural_network.h"
#include "neural_layer.h"


namespace Echo
{
	NeuralNetwork::NeuralNetwork()
		: m_isInit(false)
		, m_activationFunction(nullptr)
		, m_activationFunctionPrime(nullptr)
		, m_lossFunctionPrime(nullptr)
		, m_learningRate(0.01f)
	{
		setActivationFunction(nn::sigmoid);
		setActivationFunctionPrime(nn::sigmoid_prime);
		setLossFunctionPrime(nn::squaredErrorPrime);
	}

	void NeuralNetwork::bindMethods()
	{
		CLASS_BIND_METHOD(NeuralNetwork, train,			  DEF_METHOD("train"));
		CLASS_BIND_METHOD(NeuralNetwork, computeOutput,   DEF_METHOD("computeOutput"));
		CLASS_BIND_METHOD(NeuralNetwork, getLearningRate, DEF_METHOD("getLearningRate"));
		CLASS_BIND_METHOD(NeuralNetwork, setLearningRate, DEF_METHOD("setLearningRate"));
		CLASS_BIND_METHOD(NeuralNetwork, reset,			  DEF_METHOD("reset"));

		CLASS_REGISTER_PROPERTY(NeuralNetwork, "LearningRate", Variant::Type::Real, "getLearningRate", "setLearningRate");
	}

	// train
	void NeuralNetwork::train(const Matrix& inputVector, const Matrix& expectedOutput)
	{
		// build data structure or sync data
		organzieStructureBaseOnNodeTree();

		// compute output
		computeOutput(inputVector);

		// learn
		learn( expectedOutput);
	}

	i32 NeuralNetwork::getLayerNumber()
	{
		i32 layerNumber = 0;
		for (Node* child : getChildren())
		{
			if (child->getClassName() == "NeuralLayer")
				layerNumber++;
		}

		return layerNumber;
	}

	NeuralLayer* NeuralNetwork::getLayer(i32 layer)
	{
		i32 layerNumber = 0;
		for (Node* child : getChildren())
		{
			if (child->getClassName() == "NeuralLayer" && (layerNumber == layer))
				return ECHO_DOWN_CAST<NeuralLayer*>(child);

			layerNumber++;
		}

		return nullptr;
	}

	i32 NeuralNetwork::getNeuronNum(i32 layerIdx)
	{
		NeuralLayer* layer = getLayer(layerIdx);
		if (layer)
		{
			return layer->getNeuralNumber();
		}

		return 0;
	}

	// neuron
	Neuron* NeuralNetwork::getNeuron(i32 layer, i32 idx)
	{
		return nullptr;
	}

	// reset
	void NeuralNetwork::reset()
	{
		m_isInit = false;
	}

	// organize by node tree structure
	void NeuralNetwork::organzieStructureBaseOnNodeTree()
	{
		// init matrixs
		if (!m_isInit)
		{
			m_layerValues.clear();
			m_weights.clear();
			m_bias.clear();

			i32 layerNumber = getLayerNumber();
			if (layerNumber >= 3)
			{
				m_layerValues.resize(layerNumber);
				m_weights.resize(layerNumber - 1);
				m_dJdWeights.resize(layerNumber - 1);
				m_bias.resize(layerNumber - 1);
				m_dJdBias.resize(layerNumber - 1);

				// prepare input layer matrix
				m_layerValues[0] = Matrix( 1, getNeuronNum(0));

				// iterate
				for (i32 layerIdx = 1; layerIdx < layerNumber; layerIdx++)
				{
					i32 preNeuralNumber = getNeuronNum(layerIdx - 1);
					i32 neuralNumber = getNeuronNum(layerIdx);

					m_layerValues[layerIdx] = Matrix(1, neuralNumber);
					m_weights[layerIdx - 1] = Matrix(preNeuralNumber, neuralNumber);
					m_dJdWeights[layerIdx - 1] = Matrix(preNeuralNumber, neuralNumber);
					m_bias[layerIdx - 1] = Matrix(1, neuralNumber);
					m_dJdBias[layerIdx - 1] = Matrix(1, neuralNumber);

					m_weights[layerIdx - 1] = m_weights[layerIdx - 1].applyFunction(nn::random);
					m_bias[layerIdx - 1] = m_bias[layerIdx - 1].applyFunction(nn::random);
				}

				m_isInit = true;
			}
		}

		// sync data to neuron
	}

	// compute output by layer
	Matrix NeuralNetwork::computeLayerOutput(i32 layer, MatrixFunction fun)
	{
		return m_layerValues[layer].dot(m_weights[layer]).add(m_bias[layer]).applyFunction(fun);
	}

	// compute output
	Matrix NeuralNetwork::computeOutput(const Matrix& inputVector)
	{
		// set input layer value (row matrix)
		m_layerValues[0] = inputVector;

		i32 layerNumbr = (i32)m_layerValues.size() - 1;
		for (i32 i = 0; i < layerNumbr; i++)
		{
			m_layerValues[i + 1] = computeLayerOutput( i, m_activationFunction);
		}

		return m_layerValues[layerNumbr];
	}

	// learn
	void NeuralNetwork::learn(const Matrix& expectedOutput)
	{
		i32 layerNumber = getLayerNumber() - 1;

		// last hidden layer
		i32 lastLayer = layerNumber - 1;
		{
			m_dJdBias[lastLayer] = (*m_lossFunctionPrime)(expectedOutput, m_layerValues[lastLayer+1]).multiply(computeLayerOutput( lastLayer, m_activationFunctionPrime));
			m_dJdWeights[lastLayer] = m_layerValues[lastLayer].transpose().dot(m_dJdBias[lastLayer]);
		}

		// recursive layer
		for (i32 i = lastLayer-1; i >= 0; i--)
		{
			m_dJdBias[i] = m_dJdBias[i+1].dot(m_weights[i+1].transpose()).multiply(computeLayerOutput( i, m_activationFunctionPrime));
			m_dJdWeights[i] = m_layerValues[i].transpose().dot(m_dJdBias[i]);
		}

		// update params
		for (i32 i = 0; i < layerNumber; i++)
		{
			m_weights[i] = m_weights[i].substract( m_dJdWeights[i].multiply(m_learningRate));
			m_bias[i] = m_bias[i].substract(m_dJdBias[i].multiply(m_learningRate));
		}
	}

	// update
	void NeuralNetwork::update_self()
	{

	}
}