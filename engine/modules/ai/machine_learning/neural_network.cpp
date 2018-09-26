#include "function/activation.h"
#include "neural_network.h"
#include "neural_layer.h"


namespace Echo
{
	NeuralNetwork::NeuralNetwork()
		: m_isInit(false)
		, m_activationFunction(nullptr)
		, m_lossFunction(nullptr)
		, m_learningRate(0.001f)
	{
		setActivationFunction(nn::sigmoid);
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

	// compute output
	Matrix NeuralNetwork::computeOutput(const Matrix& inputVector)
	{
		// set input layer value (row matrix)
		m_layerValues[0] = inputVector;

		i32 layerNumbr = (i32)m_layerValues.size()-1;
		for (i32 i = 0; i < layerNumbr; i++)
		{
			m_layerValues[i + 1] = m_layerValues[i].multiply(m_weights[i]).add(m_bias[i]).applyFunction(m_activationFunction);
		}

		return m_layerValues[layerNumbr];
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
				m_bias.resize(layerNumber - 1);

				// prepare input layer matrix
				m_layerValues[0] = Matrix( 1, getNeuronNum(0));

				// iterate
				for (i32 layerIdx = 1; layerIdx < layerNumber; layerIdx++)
				{
					i32 preNeuralNumber = getNeuronNum(layerIdx - 1);
					i32 neuralNumber = getNeuronNum(layerIdx);

					m_layerValues[layerIdx] = Matrix(1, neuralNumber);
					m_weights[layerIdx - 1] = Matrix(preNeuralNumber, neuralNumber);
					m_bias[layerIdx - 1] = Matrix(1, neuralNumber);
				}

				m_isInit = true;
			}
		}

		// sync data to neuron
	}

	// learn
	void NeuralNetwork::learn(const Matrix& expectedOutput)
	{
		Matrix YStar = Matrix(expectedOutput);
	}
}